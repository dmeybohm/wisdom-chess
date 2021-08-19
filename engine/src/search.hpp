#ifndef WISDOM_CHESS_SEARCH_H_
#define WISDOM_CHESS_SEARCH_H_

#include "global.hpp"
#include "move.hpp"
#include "move_timer.hpp"
#include "search_result.hpp"

namespace wisdom::analysis
{
    class Analytics;
    class Iteration;
    class Decision;
    class Position;
}

namespace wisdom
{
    class IterativeSearchImpl;

    class Board;
    class Logger;
    class History;

    // Get the score for checkmate in X moves.
    int checkmate_score_in_moves (int moves);

    // Whether the score indicates a checkmate of the opponent has been found.
    bool is_checkmating_opponent_score (int score);

    class IterativeSearch
    {
    private:
        std::unique_ptr<IterativeSearchImpl> impl;

    public:
        IterativeSearch (Board &board, History &history, Logger &output, MoveTimer timer,
                         int total_depth);

        IterativeSearch (Board &board, History &history, Logger &output,
                         analysis::Analytics &analytics, MoveTimer timer, int total_depth);

        ~IterativeSearch ();

        SearchResult iteratively_deepen (Color side);
    };
}

#endif // WISDOM_CHESS_SEARCH_H_
