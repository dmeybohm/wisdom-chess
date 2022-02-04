#ifndef EVOLVE_CHESS_EVALUATE_H
#define EVOLVE_CHESS_EVALUATE_H

#include "piece.h"
#include "move.h"

struct board;

int    evaluate (struct board *board, color_t who, int examine_checkmate,
                 move_t *move);

#endif /* EVOLVE_CHESS_EVALUATE_H */
