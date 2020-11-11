#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

#include "global.h"
#include "move.h"

struct move_tree;

// Whether this move was a legal move for the player.
bool    was_legal_move      (struct board *board, enum color who, move_t mv);

// check if the the king indicated by the WHO argument is in trouble
// in this position
bool    is_king_threatened  (struct board *board, enum color who, uint8_t row,
                             uint8_t col);

// Whether the board is in a checkmated position for the player.
bool    is_checkmated       (struct board *board, enum color who);

// Whether this move could cause a draw.
bool    is_drawing_move     (const struct move_tree *history, move_t move);

#endif // EVOLVE_CHESS_CHECK_H
