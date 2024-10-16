#pragma once

#include <QObject>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/piece.hpp"

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

    enum class DrawByRepetitionStatus
    {
        NotReached,
        Proposed,
        Accepted,
        Declined
    };

    Q_ENUM_NS (DrawByRepetitionStatus)

    // Register the enums in QML:
    void registerQmlTypes();

    [[nodiscard]] constexpr auto 
    mapColor (wisdom::Color color) 
        -> wisdom::ui::Color
    {
        switch (color)
        {
            case wisdom::Color::White:
                return ui::Color::White;
            case wisdom::Color::Black:
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
        switch (color)
        {
            case ui::Color::White:
                return wisdom::Color::White;
            case ui::Color::Black:
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
        switch (player)
        {
            case wisdom::Player::Human:
                return ui::Player::Human;
            case wisdom::Player::ChessEngine:
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
        switch (player)
        {
            case ui::Player::Human:
                return wisdom::Player::Human;
            case ui::Player::Computer:
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
        switch (piece)
        {
            case PieceType::None:
                return wisdom::Piece::None;
            case PieceType::Pawn:
                return wisdom::Piece::Pawn;
            case PieceType::Knight:
                return wisdom::Piece::Knight;
            case PieceType::Bishop:
                return wisdom::Piece::Bishop;
            case PieceType::Rook:
                return wisdom::Piece::Rook;
            case PieceType::Queen:
                return wisdom::Piece::Queen;
            case PieceType::King:
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
        switch (piece)
        {
            case wisdom::Piece::None:
                return PieceType::None;
            case wisdom::Piece::Pawn:
                return PieceType::Pawn;
            case wisdom::Piece::Knight:
                return PieceType::Knight;
            case wisdom::Piece::Bishop:
                return PieceType::Bishop;
            case wisdom::Piece::Rook:
                return PieceType::Rook;
            case wisdom::Piece::Queen:
                return PieceType::Queen;
            case wisdom::Piece::King:
                return PieceType::King;
            default:
                assert (0);
                abort();
        }
    }
};

