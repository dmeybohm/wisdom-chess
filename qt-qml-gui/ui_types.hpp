#ifndef WISDOM_CHESSCOLOR_H
#define WISDOM_CHESSCOLOR_H

#include <QObject>

#include "piece.hpp"
#include "game.hpp"

namespace wisdom::ui
{
    Q_NAMESPACE

    enum class Color
    {
        White,
        Black,
    };

    Q_ENUM_NS(Color)

    enum class Player
    {
        Human,
        Computer
    };

    Q_ENUM_NS(Player)

    void registerQmlTypes();

    constexpr auto mapColor(wisdom::Color color) -> wisdom::ui::Color
    {
        switch (color) {
        case wisdom::Color::White: return ui::Color::White;
        case wisdom::Color::Black: return ui::Color::Black;
        default: assert(0); abort();
        }
    }

    constexpr auto mapColor(wisdom::ui::Color color) -> wisdom::Color
    {
        switch (color) {
        case ui::Color::White: return wisdom::Color::White;
        case ui::Color::Black: return wisdom::Color::Black;
        default: assert(0); abort();
        }
    }

    constexpr auto mapPlayer(wisdom::Player player) -> ui::Player
    {
        switch (player) {
        case wisdom::Player::Human: return ui::Player::Human;
        case wisdom::Player::ChessEngine: return ui::Player::Computer;
        default: assert(0); abort();
        }
    }

    constexpr auto mapPlayer(ui::Player player) -> wisdom::Player
    {
        switch (player) {
        case ui::Player::Human: return wisdom::Player::Human;
        case ui::Player::Computer: return wisdom::Player::ChessEngine;
        default: assert(0); abort();
        }
    }

};

#endif // WISDOM_CHESSCOLOR_H
