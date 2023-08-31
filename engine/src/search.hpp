#ifndef WISDOM_CHESS_SEARCH_HPP
#define WISDOM_CHESS_SEARCH_HPP

#include "global.hpp"
#include "move.hpp"
#include "move_timer.hpp"
#include "generate.hpp"

namespace wisdom
{
    class IterativeSearchImpl;

    class Board;
    class Logger;
    class History;

    struct SearchResult
    {
        int score = -Initial_Alpha;
        int depth { 0 };
        optional<Move> move { nullopt };
        bool timed_out { false };
    };

    class IterativeSearch
    {
    private:
        unique_ptr<IterativeSearchImpl> impl;

    public:
        IterativeSearch (const Board& board, const History& history, const Logger& output,
                         MoveTimer timer, int total_depth);

        ~IterativeSearch();

        [[nodiscard]] auto iterativelyDeepen (Color side) -> SearchResult;

        [[nodiscard]] auto isCancelled() -> bool;
    };
}

#endif // WISDOM_CHESS_SEARCH_HPP
