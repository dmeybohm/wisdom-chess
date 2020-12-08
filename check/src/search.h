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

class history;
struct move_timer;

// Find the best move.
move_t find_best_move (board &board, Color side, wisdom::output &output,
                       history &history);

// Iterate over each depth level.
move_t iterate (board &board, Color side, wisdom::output &output,
                history &history, move_timer &timer, int depth);

// Search for the best move to a particular depth.
search_result_t search (board &board, Color side, wisdom::output &output,
                        history &history, move_timer &timer, int depth,
                        int start_depth, int alpha, int beta, std::unique_ptr<move_tree_t> &variation);

// Get the score for checkmate in X moves.
int checkmate_score_in_moves (int moves);

#endif // EVOLVE_CHESS_SEARCH_H_
