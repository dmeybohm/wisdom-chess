#pragma once

#include "ui_types.hpp"
#include <QObject>

class GameSettings
{
    Q_GADGET

    Q_PROPERTY (wisdom::ui::Player whitePlayer MEMBER my_white_player READ whitePlayer)
    Q_PROPERTY (wisdom::ui::Player blackPlayer MEMBER my_black_player READ blackPlayer)
    Q_PROPERTY (int maxDepth MEMBER my_max_depth READ maxDepth)
    Q_PROPERTY (int maxSearchTime MEMBER my_max_search_time READ maxSearchTime)

public:
    friend bool operator== (const GameSettings&, const GameSettings&);
    friend bool operator!= (const GameSettings&, const GameSettings&);

    [[nodiscard]] auto whitePlayer() const -> wisdom::ui::Player;

    [[nodiscard]] auto blackPlayer() const -> wisdom::ui::Player;

    [[nodiscard]] auto maxDepth() const -> int;

    [[nodiscard]] auto maxSearchTime() const -> int;

private:
    wisdom::ui::Player my_white_player = wisdom::ui::Player::Human;
    wisdom::ui::Player my_black_player = wisdom::ui::Player::Computer;
    int my_max_depth = 3;
    int my_max_search_time = 4;
};

