#ifndef WISDOM_CHESS_GAMESETTINGS_H
#define WISDOM_CHESS_GAMESETTINGS_H

#include <QObject>
#include "ui_types.hpp"

class GameSettings
{
    Q_GADGET

    Q_PROPERTY(wisdom::ui::Player whitePlayer MEMBER myWhitePlayer READ whitePlayer)
    Q_PROPERTY(wisdom::ui::Player blackPlayer MEMBER myBlackPlayer READ blackPlayer)
    Q_PROPERTY(int maxDepth MEMBER myMaxDepth READ maxDepth)
    Q_PROPERTY(int maxSearchTime MEMBER myMaxSearchTime READ maxSearchTime)

public:    
    friend bool operator == (const GameSettings&, const GameSettings&);
    friend bool operator != (const GameSettings&, const GameSettings&);

    [[nodiscard]] auto whitePlayer() const -> wisdom::ui::Player;

    [[nodiscard]] auto blackPlayer() const -> wisdom::ui::Player;

    [[nodiscard]] auto maxDepth() const -> int;

    [[nodiscard]] auto maxSearchTime() const -> int;

private:
    wisdom::ui::Player myWhitePlayer = wisdom::ui::Player::Human;
    wisdom::ui::Player myBlackPlayer = wisdom::ui::Player::Computer;
    int myMaxDepth = 3;
    int myMaxSearchTime = 4;
};

#endif // WISDOM_CHESS_GAMESETTINGS_H
