#ifndef WISDOM_CHESS_SEARCH_H_
#define WISDOM_CHESS_SEARCH_H_

#include <memory>

#include "board.hpp"
#include "piece.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "move_history.hpp"
#include "output.hpp"
#include "move_timer.hpp"

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

    // Find the best move using default algorithm.
    Move find_best_move (Board &board, Color side, Output &output, History &history);

    // Find the best move using multiple threads.
    Move find_best_move_multithreaded (Board &board, Color side, Output &output, History &history);

    // Iterate over each depth level.
    SearchResult iterate (Board &board, Color side, Output &output,
                          History &history, MoveTimer &timer, int depth,
                          std::unique_ptr<MoveTree> &variation);

    // Search for the best move to a particular depth.
    SearchResult search (Board &board, Color side, Output &output,
                         History &history, MoveTimer &timer, int depth,
                         int start_depth, int alpha, int beta,
                         std::unique_ptr<MoveTree> &variation);

    // Get the score for checkmate in X moves.
    int checkmate_score_in_moves (int moves);

    bool is_checkmating_opponent_score (int score);

    class IterativeSearch
    {
    private:
        Board &my_board;
        History &my_history;
        Output &my_output;
        MoveTimer my_timer;
        int my_total_depth;

    public:
        IterativeSearch (Board &board,
                         History &history,
                         Output &output,
                         MoveTimer timer,
                         int total_depth) :
            my_board { board },
            my_history { history },
            my_output { output },
            my_timer { timer },
            my_total_depth { total_depth }
        {}

        SearchResult iteratively_deepen (Color side, std::unique_ptr<MoveTree> &variation);
    };
}

#endif // WISDOM_CHESS_SEARCH_H_
