#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

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

    // DrawByRepetitionStatus is declared in the view-model library, which
    // does not use Qt, and moc only registers enums it sees declared. QML
    // looks up DrawByRepetitionStatus.Proposed by key in this namespace's
    // meta-object, so this mirror supplies the keys with the same values.
    enum class QmlDrawByRepetitionStatus
    {
        NotReached = static_cast<int> (DrawByRepetitionStatus::NotReached),
        Proposed = static_cast<int> (DrawByRepetitionStatus::Proposed),
        Accepted = static_cast<int> (DrawByRepetitionStatus::Accepted),
        Declined = static_cast<int> (DrawByRepetitionStatus::Declined),
    };

    Q_ENUM_NS (QmlDrawByRepetitionStatus)

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

// QML sees the enums above under these names. Each is a namespace that
// registers wisdom::ui's meta-object under its own name in the WisdomChess
// module, so the same enum keys are reachable as Color.White,
// Player.Human, PieceType.Queen and DrawByRepetitionStatus.Proposed.
namespace wisdom::ui::qml::color
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE (wisdom::ui)
    QML_NAMED_ELEMENT (Color)
}

namespace wisdom::ui::qml::player
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE (wisdom::ui)
    QML_NAMED_ELEMENT (Player)
}

namespace wisdom::ui::qml::piece_type
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE (wisdom::ui)
    QML_NAMED_ELEMENT (PieceType)
}

namespace wisdom::ui::qml::draw_by_repetition_status
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE (wisdom::ui)
    QML_NAMED_ELEMENT (DrawByRepetitionStatus)
}
