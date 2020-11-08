#ifndef EVOLVE_CHESS_SEARCH_H_
#define EVOLVE_CHESS_SEARCH_H_

#include "board.h"
#include "piece.h"
#include "move.h"
#include "move_tree.h"

struct timer;

move_t  find_best_move   (struct board *board, color_t side, 
                          move_tree_t *history);

move_t iterate (struct board *board, color_t side,
                move_tree_t *history, struct timer *timer, int depth);

int search (struct board *board, color_t side, int depth, int start_depth,
            move_t *ret, int alpha, int beta, unsigned long pseudo_rand,
            move_tree_t **ret_variation, int no_quiesce, struct timer *timer,
            move_tree_t *history);

int quiesce (struct board *board, color_t side, int alpha, int beta, int depth,
             struct timer *timer, move_tree_t *history);


void print_reverse_recur (move_tree_t *tree);

#define INFINITE  	65536

#endif // EVOLVE_CHESS_SEARCH_H_
