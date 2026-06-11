#include "Gem.h"
#include "Board.h"
#include <algorithm>

// Создаёт обычный цветной гем
std::unique_ptr<Gem> GemFactory::createNormal(int color) {
    return std::make_unique<NormalGem>(color);
}

// Создаёт случайный обычный гем
std::unique_ptr<Gem> GemFactory::createRandom(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, Board::NUM_COLORS - 1);
    return std::make_unique<NormalGem>(dist(rng));
}

// Создаёт бонусный гем нужного типа
std::unique_ptr<Gem> GemFactory::createBonus(BonusType type, int srcColor) {
    if (type == BonusType::Recolor)
        return std::make_unique<RecolorBonus>(srcColor);
    return std::make_unique<BombBonus>(srcColor);
}

// Эффект перекраски:
// восстанавливает клетку приземления в исходном цвете,
// затем перекрашивает 2 несоседних клетки в радиусе 3.
void RecolorBonus::activate(Board& board, int row, int col) {
    // Возвращаем клетку приземления в исходный цвет
    board.setGem(row, col, GemFactory::createNormal(srcColor_));

    // Собираем клетки в радиусе 3, которые НЕ соседние с точкой приземления
    auto candidates = board.getCellsInRadius(row, col, 3);
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
            [&](auto p) {
                return board.isAdjacent(row, col, p.first, p.second);
            }),
        candidates.end());

    std::shuffle(candidates.begin(), candidates.end(), board.getRng());
    const int count = std::min(2, static_cast<int>(candidates.size()));
    for (int i = 0; i < count; ++i)
        board.setColor(candidates[i].first, candidates[i].second, srcColor_);
}

// Эффект бомбы:
// засчитываем очки за клетку приземления (уже пустую),
// затем уничтожаем ещё 4 случайных обычных гема.
void BombBonus::activate(Board& board, int row, int col) {
    board.addScore(10);

    // Собираем все живые обычные гемы
    std::vector<std::pair<int,int>> alive;
    for (int r = 0; r < Board::ROWS; ++r)
        for (int c = 0; c < Board::COLS; ++c) {
            Gem* g = board.getGem(r, c);
            if (g && !g->isBonus())
                alive.push_back({r, c});
        }

    std::shuffle(alive.begin(), alive.end(), board.getRng());
    const int toKill = std::min(4, static_cast<int>(alive.size()));
    for (int i = 0; i < toKill; ++i)
        board.destroyAt(alive[i].first, alive[i].second);
}
