#ifndef EVOLVE_CHESS_SEARCH_H_
#define EVOLVE_CHESS_SEARCH_H_

#include "board.h"
#include "piece.h"
#include "move.h"
#include "move_tree.h"
#include "move_history.hpp"

constexpr int INFINITE = 65536;
constexpr int INITIAL_ALPHA = INFINITE * 3;

struct search_result_t
{
    move_t move = null_move;
    int score = -INITIAL_ALPHA;
    int depth = 0;
};

struct timer;

move_t  find_best_move   (struct board *board, enum color side,
                          move_history_t &history);

move_t iterate (struct board *board, enum color side,
                move_history_t &move_history, struct timer &timer, int depth);

search_result_t search (struct board *board, enum color side, int depth, int start_depth,
                        int alpha, int beta, unsigned long pseudo_rand,
                        move_tree_t **ret_variation, int no_quiesce, struct timer &timer,
                        move_history_t &history);

int quiesce (struct board *board, enum color side, int alpha, int beta, int depth,
             struct timer *timer, move_history_t &history);

// Get the score for checkmate in X moves.
int  checkmate_score_in_moves (size_t moves);

void print_reverse_recur (move_tree_t *tree);


#endif // EVOLVE_CHESS_SEARCH_H_
