#ifndef EVOLVE_CHESS_EVALUATE_H
#define EVOLVE_CHESS_EVALUATE_H

#include "piece.h"
#include "move.h"
#include "move_tree.h"
#include "move_history.hpp"

struct board;

// Evaluate the board.
int evaluate                (board &board, Color who, int moves_away);

// Evaluate the board and check if it's a draw (the latter being a time
// consuming process potentially).
int evaluate_and_check_draw (board &board, Color who, int moves_away,
                             move_t move, const move_history_t &history);

#endif // EVOLVE_CHESS_EVALUATE_H
