#pragma once

#include <optional>

#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/piece.hpp"

namespace wisdom::ui
{
    struct PieceStep
    {
        Coord src;
        Coord dst;
    };

    // How a move changes the pieces on the board, for a frontend that
    // keeps a list of them. The captured piece is removed before the
    // others move, as it may stand on the mover's destination.
    struct PieceMovement
    {
        PieceStep mover;
        std::optional<PieceStep> castling_rook;
        std::optional<Coord> captured;

        // Piece::None unless the mover is promoted.
        Piece promoted_piece;
    };

    [[nodiscard]] auto
    pieceMovement (Move move)
        -> PieceMovement;
}
