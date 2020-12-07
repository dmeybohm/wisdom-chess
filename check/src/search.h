#ifndef EVOLVE_CHESS_SEARCH_H_
#define EVOLVE_CHESS_SEARCH_H_

#include <memory>

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

namespace wisdom
{
    class output;
}

struct move_timer;

move_t find_best_move (struct board &board, Color side, wisdom::output &output, move_history_t &move_history);

move_t iterate (struct board &board, Color side, wisdom::output &output,
                move_history_t &move_history, struct move_timer &timer, int depth);

search_result_t search (struct board &board, Color side, wisdom::output &output, int depth, int start_depth, int alpha, int beta,
        std::unique_ptr<move_tree_t> &variation, struct move_timer &timer, move_history_t &history);

// Get the score for checkmate in X moves.
int  checkmate_score_in_moves (int moves);

void print_reverse_recur (move_tree_t *tree);

#endif // EVOLVE_CHESS_SEARCH_H_
