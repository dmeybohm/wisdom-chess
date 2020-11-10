#ifndef EVOLVE_CHESS_SEARCH_H_
#define EVOLVE_CHESS_SEARCH_H_

#include "board.h"
#include "piece.h"
#include "move.h"
#include "move_tree.h"

struct timer;

move_t  find_best_move   (struct board *board, enum color side,
                          move_tree_t *history);

move_t iterate (struct board *board, enum color side,
                move_tree_t *history, struct timer *timer, int depth);

int search (struct board *board, enum color side, int depth, int start_depth,
            move_t *ret, int alpha, int beta, unsigned long pseudo_rand,
            move_tree_t **ret_variation, int no_quiesce, struct timer *timer,
            move_tree_t *history);

int quiesce (struct board *board, enum color side, int alpha, int beta, int depth,
             struct timer *timer, move_tree_t *history);

// Get the score for checkmate in X moves.
int  checkmate_score_in_moves (size_t moves);

void print_reverse_recur (move_tree_t *tree);


#define INFINITE  	   65536
#define INITIAL_ALPHA  (INFINITE * 3)

#endif // EVOLVE_CHESS_SEARCH_H_
