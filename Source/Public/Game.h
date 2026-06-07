#pragma once
#include <SFML/Graphics.hpp>
#include "Board.h"
#include <vector>
#include <string>

enum class GameState { Idle, Selected };

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;
    sf::Font         font;
    bool             fontLoaded = false;

    // game state 
    Board     board;
    GameState state  = GameState::Idle;
    int       selRow = -1;
    int       selCol = -1;

    // bonus visual 
    struct Flash {
        int       row, col;
        float     timer;      // seconds remaining
        BonusType type;
    };
    std::vector<Flash> flashes;

    // layout constants 
    static constexpr int CELL_SIZE      = 64;
    static constexpr int MARGIN         = 24;
    static constexpr int TOP_UI_HEIGHT  = 56;

    // helpers
    void handleEvent(const sf::Event& ev);
    void handleClick(int mx, int my);
    void processFullTurn();

    void render(float dt);
    void drawBackground();
    void drawCell(int r, int c, float dt);
    void drawFlashes();
    void drawUI();

    sf::Color        cellColor(int idx) const;
    std::pair<int,int> screenToGrid(int x, int y) const;
    sf::Vector2f       gridToScreen(int r, int c) const;

    bool tryLoadFont();
};
