#ifndef WISDOM_PERFT_HPP
#define WISDOM_PERFT_HPP

#include "global.hpp"

namespace wisdom
{
    class Board;
    class MoveList;
    enum class Color;
    struct Move;
}

namespace wisdom::perft
{
    using string = std::string;

    struct MoveCounter
    {
        int64_t nodes = 0;
        int64_t captures = 0;
        int64_t en_passants = 0;

        void operator+= (const MoveCounter &src)
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
        std::vector<PerftMoveResult> move_results {};
    };

    struct Stats
    {
        MoveCounter counters;

        void search_moves (wisdom::Board &board, wisdom::Color side,
                           int depth, int max_depth);

        void operator+= (const Stats &source)
        {
            counters += source.counters;
        }
    };

    // Convert a perft move list to a wisdom::MoveList.
    auto to_move_list (const wisdom::Board &board, Color who,
                       const string &move_list) -> wisdom::MoveList;

    // Convert a wisdom move to a perft move.
    auto to_perft_move (const Move &move, Color who) -> string;

    // Convert a perft move to a wisdom::Move.
    auto convert_move (const Board &board, Color who,
                       string move_str) -> wisdom::Move;

    // Output the perf results to a string.
    auto perft_results (const Board &board, Color active_player, int depth)
        -> PerftResults;

    // Apply the list of moves, update active color and return it.
    auto apply_list (Board &board, Color who, const MoveList &list) -> Color;

    // Convert the PerftResults to a string.
    auto to_string (const PerftResults &perft_results) -> string;
}

#endif // WISDOM_PERFT_HPP
