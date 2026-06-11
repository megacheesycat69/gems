#pragma once
#include <memory>
#include <random>

// Предварительное объявление, чтобы избежать циклического включения с Board.h
class Board;

// Абстрактный базовый класс — каждая клетка на поле является Gem
class Gem {
public:
    virtual ~Gem() = default;

    virtual int  color()   const = 0;
    virtual bool isBonus() const { return false; }

    // Бонусные гемы переопределяют этот метод для применения эффекта.
    // Вызывается из Board::activateBonuses(); к моменту вызова
    // гем уже извлечён из сетки, поэтому клетка пуста.
    virtual void activate(Board& board, int row, int col) {}

    virtual std::unique_ptr<Gem> clone() const = 0;
};

// Обычный цветной гем
class NormalGem : public Gem {
    int color_;
public:
    explicit NormalGem(int c) : color_(c) {}

    int  color()  const override { return color_; }
    void setColor(int c)         { color_ = c; }

    std::unique_ptr<Gem> clone() const override {
        return std::make_unique<NormalGem>(color_);
    }
};

// Перечисление типов бонусов
enum class BonusType { Recolor, Bomb };

// Абстрактный базовый класс для всех бонусных гемов
class BonusGem : public Gem {
protected:
    int srcColor_; // цвет гема, который породил этот бонус
public:
    explicit BonusGem(int srcColor) : srcColor_(srcColor) {}

    int       color()    const override { return srcColor_; }
    bool      isBonus()  const override { return true; }
    int       srcColor() const          { return srcColor_; }
    virtual BonusType getType() const = 0;
};

// Бонус перекраски — окрашивает клетку приземления и 2 несоседних в радиусе 3
class RecolorBonus : public BonusGem {
public:
    explicit RecolorBonus(int srcColor) : BonusGem(srcColor) {}

    BonusType getType() const override { return BonusType::Recolor; }
    void activate(Board& board, int row, int col) override;

    std::unique_ptr<Gem> clone() const override {
        return std::make_unique<RecolorBonus>(srcColor_);
    }
};

// Бомба — уничтожает 5 случайных гемов, включая клетку приземления
class BombBonus : public BonusGem {
public:
    explicit BombBonus(int srcColor) : BonusGem(srcColor) {}

    BonusType getType() const override { return BonusType::Bomb; }
    void activate(Board& board, int row, int col) override;

    std::unique_ptr<Gem> clone() const override {
        return std::make_unique<BombBonus>(srcColor_);
    }
};

// Фабрика гемов — централизованное создание всех типов
class GemFactory {
public:
    static std::unique_ptr<Gem> createNormal(int color);
    static std::unique_ptr<Gem> createRandom(std::mt19937& rng);
    static std::unique_ptr<Gem> createBonus(BonusType type, int srcColor);
};
