#pragma once

#include <QObject>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"

class GameSettings
{
    Q_GADGET

    Q_PROPERTY (wisdom::ui::Player whitePlayer 
            MEMBER my_white_player 
            READ whitePlayer)
    Q_PROPERTY (wisdom::ui::Player blackPlayer 
            MEMBER my_black_player 
            READ blackPlayer)
    Q_PROPERTY (int maxDepth 
            MEMBER my_max_depth 
            READ maxDepth)
    Q_PROPERTY (int maxSearchTime 
            MEMBER my_max_search_time 
            READ maxSearchTime)

public:
    friend auto 
    operator== (const GameSettings&, const GameSettings&) 
        -> bool;

    friend auto 
    operator!= (const GameSettings&, const GameSettings&) 
        -> bool;

    [[nodiscard]] auto 
    whitePlayer() const 
        -> wisdom::ui::Player;

    [[nodiscard]] auto 
    blackPlayer() const 
        -> wisdom::ui::Player;

    [[nodiscard]] auto 
    maxDepth() const 
        -> int;

    [[nodiscard]] auto 
    maxSearchTime() const 
        -> int;

private:
    wisdom::ui::Player my_white_player = wisdom::ui::Player::Human;
    wisdom::ui::Player my_black_player = wisdom::ui::Player::Computer;
    int my_max_depth = wisdom::Default_Max_Depth / 2;
    int my_max_search_time = wisdom::Default_Max_Search_Seconds;
};

