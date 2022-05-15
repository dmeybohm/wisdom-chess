#ifndef WISDOM_CHESS_SEARCH_HPP
#define WISDOM_CHESS_SEARCH_HPP

#include "global.hpp"
#include "move.hpp"
#include "move_timer.hpp"
#include "search_result.hpp"
#include "generate.hpp"

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

    class IterativeSearch
    {
    private:
        unique_ptr<IterativeSearchImpl> impl;

    public:
        IterativeSearch (Board& board, History& history, const Logger& output,
                         MoveTimer timer, int total_depth);

        IterativeSearch (Board& board, History& history, const Logger& output,
                         MoveTimer timer, int total_depth, analysis::Analytics& analytics);

        ~IterativeSearch ();

        [[nodiscard]] auto iteratively_deepen (Color side) -> SearchResult;
    };
}

#endif // WISDOM_CHESS_SEARCH_HPP
