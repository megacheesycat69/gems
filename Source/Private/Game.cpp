#include "Game.h"
#include <cmath>
#include <algorithm>
#include <string>


static const sf::Color PALETTE[] = {
    sf::Color(220,  60,  60),  
    sf::Color(60, 200,  80),   
    sf::Color(60, 120, 230),   
    sf::Color(230, 210,  50),   
    sf::Color(170,  60, 210),  
    sf::Color(50, 200, 210),   
};

static const sf::Color BONUS_RECOLOR_CLR = sf::Color(255, 230, 80);
static const sf::Color BONUS_BOMB_CLR = sf::Color(255, 120, 30);


Game::Game()
    : window(sf::VideoMode({
          static_cast<unsigned>(BOARD_COLS * CELL_SIZE + 2 * MARGIN),
          static_cast<unsigned>(BOARD_ROWS * CELL_SIZE + 2 * MARGIN + TOP_UI_HEIGHT)
        }), "GEMS")
{
    window.setFramerateLimit(60); 
    fontLoaded = tryLoadFont();
}

bool Game::tryLoadFont() {
    const char* paths[] = {
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        nullptr
    };

    for (int i = 0; paths[i]; ++i) {
        if (font.openFromFile(paths[i]))
            return true;
    }
    return false;
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else {
                handleEvent(*event);
            }
        }

        // Обновляем таймеры вспышек от бонусов
        for (auto& f : flashes) f.timer -= dt;

        flashes.erase(
            std::remove_if(flashes.begin(), flashes.end(),
                [](const Flash& f) { return f.timer <= 0.f; }),
            flashes.end());

        render(dt);
    }
}


void Game::handleEvent(const sf::Event& ev) {
    if (const auto* mb = ev.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left)
            handleClick(mb->position.x, mb->position.y);
    }
}

void Game::handleClick(int mx, int my) {
    auto [r, c] = screenToGrid(mx, my);
    const bool inBounds = (r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS);

    if (!inBounds) {
        // Кликнули мимо поля (например, в UI) – сбрасываем выделение
        state = GameState::Idle;
        selRow = selCol = -1;
        return;
    }

    if (state == GameState::Idle) {
        // Ничего не выделено -> выделяем текущую клетку
        selRow = r;  selCol = c;
        state = GameState::Selected;
        return;
    }

    // Тут state == Selected (уже есть выделенный камень)
    if (r == selRow && c == selCol) {
        state = GameState::Idle;
        selRow = selCol = -1;
    }
    else if (board.isAdjacent(selRow, selCol, r, c)) {
        // Камни соседние? Пробуем махнуть местами
        const bool ok = board.trySwap(selRow, selCol, r, c);
        state = GameState::Idle;
        selRow = selCol = -1;

        if (ok) processFullTurn(); // Если свап успешный - запускаем цепную реакцию
    }
    else {
        // Кликнули на другой камень где-то далеко – просто переносим выделение на него
        selRow = r;  selCol = c;
    }
}

// Game processing

void Game::processFullTurn() {
    // Ограничитель каскадов, чтобы игра не зависла намертво,если камни багнут и будут бесконечно складываться
    constexpr int MAX_CASCADES = 30;
    int cascades = 0;

    while (board.hasMatches() && cascades < MAX_CASCADES) {
        // Сносим собранные ряды и спавним бонусы
        auto bonuses = board.processMatches();

        // Закидываем визуальные вспышки в места, где появились бонусы
        for (const auto& b : bonuses) {
            flashes.push_back({ b.row, b.col, 0.9f, b.type });
        }

        // Камни падают вниз + досыпаем новые сверху
        board.applyGravity();
        board.refill();

        // Активируем бонусы 
        for (const auto& b : bonuses)
            board.applyBonus(b);

        // Еще раз роняем камни, так как бомбы могли наделать дыр в поле
        board.applyGravity();
        board.refill();

        window.setTitle("GEMS  —  Score: " + std::to_string(board.score));

        ++cascades;
    }
}

// Rendering
void Game::render(float dt) {
    window.clear(sf::Color(18, 18, 28));
    drawBackground();

    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            drawCell(r, c, dt);

    drawFlashes();
    drawUI();

    window.display();
}

void Game::drawBackground() {
    // Рисуем подложку игрового поля
    sf::RectangleShape bg({
        float(BOARD_COLS * CELL_SIZE + 8),
        float(BOARD_ROWS * CELL_SIZE + 8)
        });
    bg.setPosition({ float(MARGIN - 4), float(MARGIN - 4 + TOP_UI_HEIGHT) });
    bg.setFillColor(sf::Color(10, 10, 18));
    bg.setOutlineColor(sf::Color(60, 60, 90));
    bg.setOutlineThickness(2.f);
    window.draw(bg);

    // Рисуем сетку (клетки)
    for (int r = 0; r <= BOARD_ROWS; ++r) {
        sf::RectangleShape line({ float(BOARD_COLS * CELL_SIZE), 1.f });
        line.setPosition({ float(MARGIN), float(MARGIN + TOP_UI_HEIGHT + r * CELL_SIZE) });
        line.setFillColor(sf::Color(40, 40, 60));
        window.draw(line);
    }
    for (int c = 0; c <= BOARD_COLS; ++c) {
        sf::RectangleShape line({ 1.f, float(BOARD_ROWS * CELL_SIZE) });
        line.setPosition({ float(MARGIN + c * CELL_SIZE), float(MARGIN + TOP_UI_HEIGHT) });
        line.setFillColor(sf::Color(40, 40, 60));
        window.draw(line);
    }
}

void Game::drawCell(int r, int c, float /*dt*/) {
    const int colour = board.getColor(r, c);
    if (colour == -1) return; // Пустая клетка (по идее такого быть не должно, но мало ли)

    const bool selected = (r == selRow && c == selCol);
    const sf::Vector2f pos = gridToScreen(r, c);
    constexpr float PAD = 3.f; // Отступ от краев клетки
    constexpr float SIZE = CELL_SIZE - PAD * 2;

    //просто рисуем полупрозрачный черный квадрат с небольшим смещением
    sf::RectangleShape shadow({ SIZE, SIZE });
    shadow.setPosition(pos + sf::Vector2f(PAD + 2, PAD + 2));
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    window.draw(shadow);

    // Основное тело камня
    sf::RectangleShape gem({ SIZE, SIZE });
    gem.setPosition(pos + sf::Vector2f(PAD, PAD));

    sf::Color base = cellColor(colour);
    if (selected) {
        // Подсвечиваем выделенный камень
        base.r = static_cast<uint8_t>(std::min(255, (int)base.r + 60));
        base.g = static_cast<uint8_t>(std::min(255, (int)base.g + 60));
        base.b = static_cast<uint8_t>(std::min(255, (int)base.b + 60));
    }
    gem.setFillColor(base);

    if (selected) {
        gem.setOutlineThickness(3.f);
        gem.setOutlineColor(sf::Color::White); // Белая рамка для выделенного
    }
    window.draw(gem);
    // Процедурная генерация бликов, лол.

    // Верхний блик
    const float shineW = SIZE * 0.55f;
    const float shineH = SIZE * 0.18f;
    sf::RectangleShape shine({ shineW, shineH });
    shine.setPosition(pos + sf::Vector2f(PAD + SIZE * 0.10f, PAD + SIZE * 0.10f));
    shine.setFillColor(sf::Color(255, 255, 255, 55));
    window.draw(shine);

    // Боковой левый блик (чуть тусклее)
    sf::RectangleShape shine2({ shineH * 0.6f, SIZE * 0.3f });
    shine2.setPosition(pos + sf::Vector2f(PAD + SIZE * 0.10f, PAD + SIZE * 0.30f));
    shine2.setFillColor(sf::Color(255, 255, 255, 30));
    window.draw(shine2);
}

void Game::drawFlashes() {
    for (const auto& f : flashes) {
        // Считаем прозрачность по таймеру 
        const float alpha = std::max(0.f, f.timer / 0.9f);
        const sf::Vector2f pos = gridToScreen(f.row, f.col);

        const sf::Color base = (f.type == BonusType::Bomb) ? BONUS_BOMB_CLR : BONUS_RECOLOR_CLR;

        // Расширяющееся светящееся кольцо (схлопывается со временем)
        const float maxR = CELL_SIZE * 0.9f;
        const float radius = maxR * (1.f - alpha) + 4.f;
        sf::CircleShape ring(radius);
        ring.setOrigin({ radius, radius }); // Центрируем
        ring.setPosition(pos + sf::Vector2f(CELL_SIZE * 0.5f, CELL_SIZE * 0.5f));
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(sf::Color(base.r, base.g, base.b, (uint8_t)(alpha * 220)));
        ring.setOutlineThickness(4.f);
        window.draw(ring);

        // Полупрозрачный квадрат поверх самого камня
        sf::RectangleShape label({ float(CELL_SIZE - 6), float(CELL_SIZE - 6) });
        label.setPosition(pos + sf::Vector2f(3.f, 3.f));
        label.setFillColor(sf::Color(base.r, base.g, base.b, (uint8_t)(alpha * 120)));
        window.draw(label);
    }
}

void Game::drawUI() {
    //подложка для интерфейса сверху
    sf::RectangleShape topBar({
        float(BOARD_COLS * CELL_SIZE + 8),
        float(TOP_UI_HEIGHT - 8)
        });
    topBar.setPosition({ float(MARGIN - 4), float(MARGIN - 4) });
    topBar.setFillColor(sf::Color(28, 28, 42));
    topBar.setOutlineColor(sf::Color(60, 60, 90));
    topBar.setOutlineThickness(2.f);
    window.draw(topBar);

    const std::string scoreStr = "Score: " + std::to_string(board.score);

    if (fontLoaded) {
        //нормальный рендер текста, если шрифт нашелся
        sf::Text scoreText(font, scoreStr, 22);
        scoreText.setFillColor(sf::Color(220, 220, 255));
        scoreText.setPosition({ float(MARGIN + 4), float(MARGIN + 6) });
        window.draw(scoreText);

        sf::Text hint(font, "Click two adjacent gems to swap", 14);
        hint.setFillColor(sf::Color(120, 120, 160));
        hint.setPosition({ float(MARGIN + 4), float(MARGIN + 32) });
        window.draw(hint);
    }
    else {
        //если шрифта нет, рисуем дурацкую полоску счета.
        const int barWidth = std::min(board.score * 3, BOARD_COLS * CELL_SIZE - 8);
        sf::RectangleShape bar({ float(barWidth), 12.f });
        bar.setPosition({ float(MARGIN + 4), float(MARGIN + 10) });
        bar.setFillColor(sf::Color(100, 200, 255));
        window.draw(bar);
    }

    // Декоративные точки в правом верхнем углу (чисто для красоты, показывает цвета игры)
    const float dotR = 7.f;
    const float startX = float(MARGIN + BOARD_COLS * CELL_SIZE - NUM_COLORS * (dotR * 2 + 6) - 4);
    for (int i = 0; i < NUM_COLORS; ++i) {
        sf::CircleShape dot(dotR);
        dot.setFillColor(cellColor(i));
        dot.setPosition({ startX + i * (dotR * 2 + 6), float(MARGIN + TOP_UI_HEIGHT / 2 - (int)dotR) });
        window.draw(dot);
    }
}

// Utilities

sf::Color Game::cellColor(int idx) const {
    if (idx < 0 || idx >= NUM_COLORS) return sf::Color::Transparent; // Заглушка от крашей
    return PALETTE[idx];
}

std::pair<int, int> Game::screenToGrid(int x, int y) const {
    // Переводим пиксели в координаты массива (отсекая марджины и шапку)
    const int c = (x - MARGIN) / CELL_SIZE;
    const int r = (y - MARGIN - TOP_UI_HEIGHT) / CELL_SIZE;
    return { r, c };
}

sf::Vector2f Game::gridToScreen(int r, int c) const {
    // Обратно: из массива в пиксели экрана
    return {
        float(MARGIN + c * CELL_SIZE),
        float(MARGIN + TOP_UI_HEIGHT + r * CELL_SIZE)
    };
}