#ifndef EVOLVE_CHESS_EVALUATE_H
#define EVOLVE_CHESS_EVALUATE_H

#include "piece.h"
#include "move.h"
#include "move_tree.h"

struct board;

int    evaluate (struct board *board, color_t who, int examine_checkmate,
                 move_t *move);

int    evaluate_and_check_draw (struct board *board, color_t who, int examine_checkmate,
                                move_t *move, move_tree_t *history);

#endif // EVOLVE_CHESS_EVALUATE_H
