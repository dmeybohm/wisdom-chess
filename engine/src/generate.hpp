#pragma once

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "move_list.hpp"

namespace wisdom
{
    [[nodiscard]] auto
    generateAllPotentialMoves (const Board& board, Color who)
        -> MoveList;

    [[nodiscard]] auto
    generateLegalMoves (const Board& board, Color who)
        -> MoveList;

    [[nodiscard]] inline auto 
    needPawnPromotion (int row, Color who) 
        -> bool
    {
        assert (isColorValid (who));
        switch (who)
        {
            case Color::White:
                return 0 == row;
            case Color::Black:
                return 7 == row;
            default:
                throw Error { "Invalid color in needPawnPromotion()" };
        }
    }

    // Return en passant column if the board is the player is eligible.
    [[nodiscard]] auto
    eligibleEnPassantColumn (const Board& board, int row, int column, Color who)
        -> optional<int>;
}

