#ifndef EVOLVE_CHESS_EVALUATE_H
#define EVOLVE_CHESS_EVALUATE_H

#include "piece.h"
#include "move.h"
#include "move_tree.h"

struct board;

// Evaluate the board.
int evaluate                (struct board *board, enum color who, size_t moves_away);

// Evaluate the board and check if it's a draw (the latter being a time
// consuming process potentially).
int evaluate_and_check_draw (struct board *board, enum color who, size_t moves_away,
                             move_t move, move_tree_t *history);

#endif // EVOLVE_CHESS_EVALUATE_H
