#ifndef EVOLVE_CHESS_EVALUATE_H
#define EVOLVE_CHESS_EVALUATE_H

#include "piece.h"
#include "move.h"
#include "move_tree.h"

struct Board;
class History;

// Evaluate the board.
int evaluate (Board &board, Color who, int moves_away);

// Evaluate the board and check if it's a draw (the latter being a time
// consuming process potentially).
int evaluate_and_check_draw (Board &board, Color who, int moves_away,
                             Move move, const History &history);

#endif // EVOLVE_CHESS_EVALUATE_H
