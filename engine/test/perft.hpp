#pragma once

#include "global.hpp"

namespace wisdom
{
    class Board;
    class MoveList;
    enum class Color : int8_t;
    struct Move;
    class MoveGenerator;
}

namespace wisdom::perft
{
    using std::string;
    using wisdom::MoveGenerator;

    struct MoveCounter
    {
        int64_t nodes = 0;
        int64_t captures = 0;
        int64_t en_passants = 0;

        void operator+= (const MoveCounter& src)
        {
            this->captures += src.captures;
            this->en_passants += src.en_passants;
            this->nodes += src.nodes;
        }
    };

    struct CounterExpectation
    {
        int depth;
        MoveCounter expectation;
    };

    struct PerftMoveResult
    {
        int64_t nodes;
        string move;
    };

    struct PerftResults
    {
        int64_t total_nodes = 0;
        vector<PerftMoveResult> move_results {};
    };

    struct Stats
    {
        MoveCounter counters;

        void searchMoves (const wisdom::Board& board, wisdom::Color side, int depth, int max_depth);

        void operator+= (const Stats& source)
        {
            counters += source.counters;
        }
    };

    // Convert a perft move list to a wisdom::MoveList.
    auto
    toMoveList (const wisdom::Board& board, Color who, const string& move_list)
        -> wisdom::MoveList;

    // Convert a wisdom move to a perft move.
    auto
    toPerftMove (const Move& move, Color who)
        -> string;

    // Convert a perft move to a wisdom::Move.
    auto
    convertMove (const Board& board, Color who, string move_str)
        -> wisdom::Move;

    // Output the perf results to a string.
    auto
    perftResults (const Board& board, Color active_player, int depth)
        -> PerftResults;

    // Apply the list of moves, update active color and return it.
    auto
    applyList (Board& board, Color color, const MoveList& list)
        -> Color;

    // Convert the PerftResults to a string.
    auto
    asString (const PerftResults& perft_results)
        -> string;
}
