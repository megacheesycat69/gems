#include "Board.h"
#include <algorithm>
#include <numeric>
#include <set>

// Construction
Board::Board() : rng(std::random_device{}()) {
    initBoard();
}

void Board::initBoard() {

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            std::set<int> forbidden;

            if (c >= 2 && grid[r][c - 1] == grid[r][c - 2])
                forbidden.insert(grid[r][c - 1]);

            if (r >= 2 && grid[r - 1][c] == grid[r - 2][c])
                forbidden.insert(grid[r - 1][c]);

            int col;
            int attempts = 0;
            do {
                col = randomColor();
                ++attempts;
            } while (forbidden.count(col) && attempts < 20);

            grid[r][c] = col;
        }
    }
}


bool Board::isAdjacent(int r1, int c1, int r2, int c2) const {
    return (r1 == r2 && std::abs(c1 - c2) == 1) ||
        (c1 == c2 && std::abs(r1 - r2) == 1);
}

int Board::getColor(int r, int c) const {
    return grid[r][c];
}

bool Board::hasMatches() const {
    auto m = findMatchedCells();
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (m[r][c]) return true;
    return false;
}

std::vector<std::vector<bool>> Board::findMatchedCells() const {
    std::vector<std::vector<bool>> visited(BOARD_ROWS, std::vector<bool>(BOARD_COLS, false));
    std::vector<std::vector<bool>> matched(BOARD_ROWS, std::vector<bool>(BOARD_COLS, false));

    const int dr[] = { 0, 0, 1, -1 };
    const int dc[] = { 1, -1, 0, 0 };

    for (int sr = 0; sr < BOARD_ROWS; ++sr) {
        for (int sc = 0; sc < BOARD_COLS; ++sc) {
            if (visited[sr][sc] || grid[sr][sc] == -1) continue; 

            const int colour = grid[sr][sc];
            std::vector<std::pair<int, int>> component;
            std::vector<std::pair<int, int>> queue = { {sr, sc} };
            visited[sr][sc] = true;

            while (!queue.empty()) {
                auto [r, c] = queue.back();
                queue.pop_back();
                component.push_back({ r, c });

                for (int d = 0; d < 4; ++d) {
                    int nr = r + dr[d];
                    int nc = c + dc[d];
                    if (nr >= 0 && nr < BOARD_ROWS &&
                        nc >= 0 && nc < BOARD_COLS &&
                        !visited[nr][nc] && grid[nr][nc] == colour) {
                        visited[nr][nc] = true;
                        queue.push_back({ nr, nc });
                    }
                }
            }
            if (component.size() >= 3) {
                for (auto [r, c] : component)
                    matched[r][c] = true;
            }
        }
    }
    return matched;
}bool Board::trySwap(int r1, int c1, int r2, int c2) {
    if (!isAdjacent(r1, c1, r2, c2)) return false;

    std::swap(grid[r1][c1], grid[r2][c2]);

    if (!hasMatches()) {
        std::swap(grid[r1][c1], grid[r2][c2]);
        return false;
    }
    return true; 
}

std::vector<BonusEvent> Board::processMatches() {
    auto matched = findMatchedCells();

    std::vector<std::tuple<int, int, int>> toDestroy;
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (matched[r][c])
                toDestroy.emplace_back(r, c, grid[r][c]);

    for (auto& [r, c, col] : toDestroy) {
        grid[r][c] = -1;
        score += 10;
    }

    std::vector<BonusEvent> bonuses;
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    std::uniform_int_distribution<int>    typeDist(0, 1);

    for (auto& [r, c, col] : toDestroy) {
        if (chance(rng) < 0.18f) {
            auto candidates = getCellsInRadius(r, c, 3);
            if (candidates.empty()) continue; 

            std::uniform_int_distribution<int> posDist(0, (int)candidates.size() - 1);
            auto [br, bc] = candidates[posDist(rng)];

            BonusType t = (typeDist(rng) == 0) ? BonusType::Recolor : BonusType::Bomb;
            bonuses.push_back({ t, br, bc, col });
        }
    }

    return bonuses;
}

void Board::applyGravity() {
    for (int c = 0; c < BOARD_COLS; ++c) {
        int write = BOARD_ROWS - 1; 
        for (int r = BOARD_ROWS - 1; r >= 0; --r) {
            if (grid[r][c] != -1) {
                grid[write][c] = grid[r][c];
                if (write != r) grid[r][c] = -1;
                --write;
            }
        }
        for (int r = write; r >= 0; --r)
            grid[r][c] = -1;
    }
}

void Board::refill() {
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (grid[r][c] == -1)
                grid[r][c] = randomColor();
}

void Board::applyBonus(const BonusEvent& ev) {
    if (ev.type == BonusType::Recolor) {
        if (grid[ev.row][ev.col] != -1)
            grid[ev.row][ev.col] = ev.srcColor;
        auto candidates = getCellsInRadius(ev.row, ev.col, 3);
        candidates.erase(
            std::remove_if(candidates.begin(), candidates.end(),
                [&](auto p) {
                    return isAdjacent(ev.row, ev.col, p.first, p.second);
                }),
            candidates.end());

        std::shuffle(candidates.begin(), candidates.end(), rng);
        const int count = std::min(2, (int)candidates.size()); 
        for (int i = 0; i < count; ++i) {
            auto [r, c] = candidates[i];
            if (grid[r][c] != -1)
                grid[r][c] = ev.srcColor;
        }

    }
    else { // Bomb
        if (grid[ev.row][ev.col] != -1) {
            grid[ev.row][ev.col] = -1;
            score += 10;
        }

        std::vector<std::pair<int, int>> alive;
        for (int r = 0; r < BOARD_ROWS; ++r)
            for (int c = 0; c < BOARD_COLS; ++c)
                if (grid[r][c] != -1)
                    alive.push_back({ r, c });

        std::shuffle(alive.begin(), alive.end(), rng);
        const int toKill = std::min(4, (int)alive.size());
        for (int i = 0; i < toKill; ++i) {
            auto [r, c] = alive[i];
            grid[r][c] = -1;
            score += 10;
        }
    }
}

int Board::randomColor() {
    std::uniform_int_distribution<int> dist(0, NUM_COLORS - 1);
    return dist(rng);
}

std::vector<std::pair<int, int>> Board::getCellsInRadius(int r, int c, int radius) const {
    std::vector<std::pair<int, int>> result;
    for (int dr = -radius; dr <= radius; ++dr) {
        for (int dc = -radius; dc <= radius; ++dc) {
            if (dr == 0 && dc == 0) continue; 
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < BOARD_ROWS &&
                nc >= 0 && nc < BOARD_COLS &&
                grid[nr][nc] != -1)
                result.push_back({ nr, nc });
        }
    }
    return result;
}
