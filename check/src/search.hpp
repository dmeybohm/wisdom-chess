#ifndef WISDOM_CHESS_SEARCH_H_
#define WISDOM_CHESS_SEARCH_H_

#include <memory>

#include "board.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "move_history.hpp"

namespace wisdom
{
    constexpr int INFINITE = 65536;
    constexpr int INITIAL_ALPHA = INFINITE * 3;

    struct SearchResult
    {
        Move move = Null_Move;
        int score = -INITIAL_ALPHA;
        int depth = 0;
    };

    class Output;

    class History;

    struct MoveTimer;

    // Find the best move.
    Move find_best_move (Board &board, Color side, Output &output, History &history);

    // Iterate over each depth level.
    Move iterate (Board &board, Color side, Output &output,
                  History &history, MoveTimer &timer, int depth);

    // Search for the best move to a particular depth.
    SearchResult search (Board &board, Color side, Output &output,
                         History &history, MoveTimer &timer, int depth,
                         int start_depth, int alpha, int beta, std::unique_ptr<MoveTree> &variation);

    // Get the score for checkmate in X moves.
    int checkmate_score_in_moves (int moves);
}

#endif // WISDOM_CHESS_SEARCH_H_
