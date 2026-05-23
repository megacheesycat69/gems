#pragma once
#include <vector>
#include <random>
#include <tuple>
#include <utility>

// Размеры поля и количество цветов. Хардкод, но пока хватает.
static constexpr int BOARD_COLS = 8;
static constexpr int BOARD_ROWS = 8;
static constexpr int NUM_COLORS = 6;

enum class BonusType { Recolor, Bomb };

// Данные о выпавшем бонусе
struct BonusEvent {
    BonusType type;
    int row, col;   // координаты, куда приземляется бонус на поле
    int srcColor;   // цвет камней, при уничтожении которых прокнул этот бонус
};

class Board {
public:
    Board();

    // Проверка, стоят ли клетки рядом (только по кресту, без диагоналей)
    bool isAdjacent(int r1, int c1, int r2, int c2) const;

    // Пробуем поменять соседние клетки. Возвращает true, если собрался ряд.
    // Если совпадений нет — свап автоматически откатывается обратно.
    bool trySwap(int r1, int c1, int r2, int c2);

    // Уничтожает группы от 3 и больше камней одного цвета (связные компоненты).
    std::vector<BonusEvent> processMatches();

    // Роняет камни вниз, заполняя пустые места в колонках
    void applyGravity();

    // Заполняет все пустые клетки новыми случайными цветами
    void refill();

    // Отрабатывает логику бонуса (взрыв или перекраска)
    void applyBonus(const BonusEvent& ev);

    // Возвращает true, если на поле есть хоть одна готовая комбинация
    bool hasMatches() const;

    int getColor(int r, int c) const;

    int score = 0; 

private:
    int grid[BOARD_ROWS][BOARD_COLS];  
    std::mt19937 rng;

    std::vector<std::vector<bool>> findMatchedCells() const;

    int randomColor();

    std::vector<std::pair<int, int>> getCellsInRadius(int r, int c, int radius) const;

    // Первоначальное заполнение. Гарантирует, что при старте игры нет готовых совпадений.
    void initBoard();
};