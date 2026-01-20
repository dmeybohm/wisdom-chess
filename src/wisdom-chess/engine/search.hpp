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
    class TranspositionTable;

    struct RankedMove
    {
        Move move {};
        int score = -Initial_Alpha;
    };

    struct SearchResult
    {
        int score = -Initial_Alpha;
        int depth { 0 };
        optional<Move> move { nullopt };
        bool timed_out { false };
        array<optional<RankedMove>, Top_Moves_Count> top_moves {};
    };

    class IterativeSearch
    {
    public:
        [[nodiscard]] static auto
        create (
            const Board& board,
            const History& history,
            shared_ptr<Logger> logger,
            const MoveTimer& timer,
            int max_depth,
            TranspositionTable& transposition_table
        ) -> IterativeSearch;

        // Copy and move constructors
        IterativeSearch (const IterativeSearch& other) = delete;
        IterativeSearch& operator= (const IterativeSearch& other) = delete;

        IterativeSearch (IterativeSearch&& other) noexcept = default;
        IterativeSearch& operator= (IterativeSearch&& other) noexcept = default;

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

        // Private constructor for factory functions
        explicit IterativeSearch (unique_ptr<IterativeSearchImpl> impl);
    };
}
