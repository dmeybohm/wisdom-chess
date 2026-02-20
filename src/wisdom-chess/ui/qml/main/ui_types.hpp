#pragma once

#include <QObject>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

namespace wisdom::ui
{
    Q_NAMESPACE

    enum class Color
    {
        White,
        Black,
    };

    Q_ENUM_NS (Color)

    enum class Player
    {
        Human,
        Computer
    };

    Q_ENUM_NS (Player)

    enum class PieceType
    {
        None = 0,
        Pawn,
        Knight,
        Bishop,
        Rook,
        Queen,
        King
    };

    Q_ENUM_NS (PieceType)

    // Register the enums in QML:
    void registerQmlTypes();

    [[nodiscard]] constexpr auto 
    mapColor (wisdom::Color color)
        -> wisdom::ui::Color
    {
        using enum wisdom::Color;
        switch (color)
        {
            case White:
                return ui::Color::White;
            case Black:
                return ui::Color::Black;
            default:
                assert (0);
                abort();
        }
    }

    [[nodiscard]] constexpr auto 
    mapColor (wisdom::ui::Color color)
        -> wisdom::Color
    {
        using enum ui::Color;
        switch (color)
        {
            case White:
                return wisdom::Color::White;
            case Black:
                return wisdom::Color::Black;
            default:
                assert (0);
                abort();
        }
    }

    [[nodiscard]] constexpr auto 
    mapPlayer (wisdom::Player player)
        -> ui::Player
    {
        using enum wisdom::Player;
        switch (player)
        {
            case Human:
                return ui::Player::Human;
            case ChessEngine:
                return ui::Player::Computer;
            default:
                assert (0);
                abort();
        }
    }

    [[nodiscard]] constexpr auto 
    mapPlayer (ui::Player player)
        -> wisdom::Player
    {
        using enum ui::Player;
        switch (player)
        {
            case Human:
                return wisdom::Player::Human;
            case Computer:
                return wisdom::Player::ChessEngine;
            default:
                assert (0);
                abort();
        }
    }

    [[nodiscard]] constexpr auto 
    mapPiece (ui::PieceType piece)
        -> wisdom::Piece
    {
        using enum PieceType;
        switch (piece)
        {
            case None:
                return wisdom::Piece::None;
            case Pawn:
                return wisdom::Piece::Pawn;
            case Knight:
                return wisdom::Piece::Knight;
            case Bishop:
                return wisdom::Piece::Bishop;
            case Rook:
                return wisdom::Piece::Rook;
            case Queen:
                return wisdom::Piece::Queen;
            case King:
                return wisdom::Piece::King;
            default:
                assert (0);
                abort();
        }
    }

    [[nodiscard]] constexpr auto 
    mapPiece (wisdom::Piece piece)
        -> ui::PieceType
    {
        using enum wisdom::Piece;
        switch (piece)
        {
            case None:
                return PieceType::None;
            case Pawn:
                return PieceType::Pawn;
            case Knight:
                return PieceType::Knight;
            case Bishop:
                return PieceType::Bishop;
            case Rook:
                return PieceType::Rook;
            case Queen:
                return PieceType::Queen;
            case King:
                return PieceType::King;
            default:
                assert (0);
                abort();
        }
    }
};

