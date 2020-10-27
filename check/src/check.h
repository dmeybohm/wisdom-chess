#ifndef EVOLVE_CHESS_CHECK_H
#define EVOLVE_CHESS_CHECK_H

#include "global.h"

struct move_tree;
struct move;

bool    was_legal_move      (struct board *board, color_t who, move_t *mv);
bool    is_king_threatened  (struct board *board, color_t who, uint8_t row,
                             uint8_t col);
bool    is_checkmated       (struct board *board, color_t who);
bool    is_drawing_move     (const struct move_tree *history, const struct move *move);

#endif // EVOLVE_CHESS_CHECK_H
