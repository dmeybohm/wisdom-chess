#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/generate.hpp"

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
    public:
        IterativeSearch (
            const Board& board,
            const History& history,
            shared_ptr<Logger> output,
            const MoveTimer& timer,
            int total_depth
        );

        ~IterativeSearch();

        [[nodiscard]] auto
        iterativelyDeepen (Color side)
            -> SearchResult;

        [[nodiscard]] auto
        isCancelled()
            -> bool;

        [[nodiscard]] auto
        moveTimer() const&
            -> const MoveTimer&;
        void moveTimer() && = delete;

    private:
        unique_ptr<IterativeSearchImpl> impl;
    };
}
