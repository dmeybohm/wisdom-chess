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
        optional<Move> move;
        int score;
        int depth;
        bool timed_out;

        static SearchResult from_initial () noexcept
        {
            SearchResult result { nullopt, -Initial_Alpha, 0, false };
            return result;
        }
    };

    class IterativeSearch
    {
    private:
        unique_ptr<IterativeSearchImpl> impl;

    public:
        IterativeSearch (const Board& board, History& history, const Logger& output,
                         MoveTimer timer, int total_depth);

        ~IterativeSearch ();

        [[nodiscard]] auto iteratively_deepen (Color side) -> SearchResult;

        [[nodiscard]] auto is_cancelled () -> bool;
    };
}

#endif // WISDOM_CHESS_SEARCH_HPP
