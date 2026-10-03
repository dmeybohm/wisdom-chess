#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/move_ordering.hpp"

namespace wisdom
{
    // Generate all potential moves including illegal moves for the player.
    [[nodiscard]] auto
    generateAllPotentialMoves (const Board& board, Color who) noexcept
        -> MoveList;

    // Generate all potential moves, with the moves the ordering names
    // sorted ahead of the rest.
    [[nodiscard]] auto
    generateAllPotentialMoves (const Board& board, Color who, const MoveOrdering& ordering) noexcept
        -> MoveList;

    // Generate the potential captures and promotions to a queen for the
    // player, including illegal moves, ordered as generateAllPotentialMoves()
    // orders them.
    [[nodiscard]] auto
    generateCaptures (const Board& board, Color who) noexcept
        -> MoveList;

    // Generate only legal moves from the board for the player.
    [[nodiscard]] auto
    generateLegalMoves (const Board& board, Color who) noexcept
        -> MoveList;

    // Generate only the legal en passant captures for the player to move.
    [[nodiscard]] auto
    generateLegalEnPassantMoves (const Board& board) noexcept
        -> MoveList;

    // Whether the player to move has at least one legal move.
    [[nodiscard]] auto
    hasLegalMove (const Board& board) noexcept
        -> bool;

    // The same, for a caller that already knows whether the player to move
    // is in check.
    [[nodiscard]] auto
    hasLegalMove (const Board& board, bool in_check) noexcept
        -> bool;

    // Whether the position reached by `who` playing `mv` is legal: the
    // mover's king is not attacked, and a castling king did not start
    // in, or pass through, check.
    [[nodiscard]] auto
    isLegalPositionAfterMove (const Board& board, Color who, Move mv) noexcept
        -> bool;

    // Whether the player to move is checkmated.
    [[nodiscard]] auto
    isCheckmated (const Board& board) noexcept
        -> bool;

    // Whether the player to move is stalemated.
    [[nodiscard]] auto
    isStalemated (const Board& board) noexcept
        -> bool;

    // Whether the pawn needs to be promoted when it arrives at the row.
    [[nodiscard]] auto
    needPawnPromotion (int row, Color who) noexcept
        -> bool;

    // Return en passant column of the board if the player is eligible.
    [[nodiscard]] auto
    eligibleEnPassantColumn (const Board& board, int row, int column, Color who) noexcept
        -> optional<int>;
}
