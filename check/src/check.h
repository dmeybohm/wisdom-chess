#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

#include "global.h"

struct move_tree;
struct move;

// Whether this move was a legal move for the player.
bool    was_legal_move      (struct board *board, color_t who, move_t *mv);

// check if the the king indicated by the WHO argument is in trouble
// in this position
bool    is_king_threatened  (struct board *board, color_t who, uint8_t row,
                             uint8_t col);

// Whether the board is in a checkmated position for the player.
bool    is_checkmated       (struct board *board, color_t who);

// Whether this move could cause a draw.
bool    is_drawing_move     (const struct move_tree *history, const struct move *move);

#endif // EVOLVE_CHESS_CHECK_H
