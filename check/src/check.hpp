#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

#include "global.hpp"
#include "move.hpp"
#include "history.hpp"

namespace wisdom
{
    struct move_tree;

// Whether this move was a legal move for the player.
    bool was_legal_move (Board &board, Color who, Move mv);

// check if the the king indicated by the WHO argument is in trouble
// in this position
    bool is_king_threatened (Board &board, Color who,
                             int8_t row, int8_t col);

    static inline bool is_king_threatened (Board &board, Color who, Coord pos)
    {
        return is_king_threatened (board, who, ROW (pos), COLUMN (pos));
    }

// Whether the board is in a checkmated position for the player.
    bool is_checkmated (Board &board, Color who);

// Whether this move could cause a draw.
    static inline bool is_drawing_move (Board &board, Color who, Move mv, const History &history)
    {
        return history.is_third_repetition (board) ||
               History::is_fifty_move_repetition (board);
    }
}

#endif // EVOLVE_CHESS_CHECK_H
