#pragma once

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "move_list.hpp"

namespace wisdom
{
    // Generate all potential moves including illegal moves for the player..
    [[nodiscard]] auto
    generateAllPotentialMoves (const Board& board, Color who)
        -> MoveList;

    // Generate only legal moves from the board for the player.
    [[nodiscard]] auto
    generateLegalMoves (const Board& board, Color who)
        -> MoveList;

    // Whether the pawn needs to be promoted when it arrives at the row.
    [[nodiscard]] auto
    needPawnPromotion (int row, Color who) 
        -> bool;

    // Return en passant column of the board if the player is eligible.
    [[nodiscard]] auto
    eligibleEnPassantColumn (const Board& board, int row, int column, Color who)
        -> optional<int>;
}
