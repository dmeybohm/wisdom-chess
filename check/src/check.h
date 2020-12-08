#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

#include "global.h"
#include "move.h"
#include "history.hpp"

struct move_tree;
class history;

// Whether this move was a legal move for the player.
bool    was_legal_move      (board &board, Color who, move_t mv);

// check if the the king indicated by the WHO argument is in trouble
// in this position
bool    is_king_threatened  (board &board, Color who,
                             int8_t row, int8_t col);

static inline bool is_king_threatened (board &board, Color who, coord_t pos)
{
    return is_king_threatened (board, who, ROW(pos), COLUMN(pos));
}

// Whether the board is in a checkmated position for the player.
bool    is_checkmated       (board &board, Color who);

// Whether this move could cause a draw.
static inline bool is_drawing_move (board &board, Color who, move_t mv, const history &history)
{
    return history.is_third_repetition (mv, who) ||
        history.is_fifty_move_repetition (board, who);
}

#endif // EVOLVE_CHESS_CHECK_H
