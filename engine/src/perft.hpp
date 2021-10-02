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
    struct MoveCounter
    {
        int nodes = 0;
        int captures = 0;
        int en_passants = 0;

        [[nodiscard]] auto operator+ (MoveCounter &) const -> MoveCounter;
    };

    struct CounterExpectation
    {
        int depth;
        MoveCounter expectation;
    };

    struct Stats
    {
        MoveCounter counters;

        void search_moves (wisdom::Board &board, wisdom::Color side,
                           int depth, int max_depth);

        [[nodiscard]] auto operator+ (Stats &source) const -> Stats;
    };

    // Convert a perft move list to a wisdom::MoveList.
    auto to_move_list (const wisdom::Board &board, Color who,
                       const std::string &move_list) -> wisdom::MoveList;

    // Convert a wisdom move to a perft move.
    auto to_perft_move (const Move &move, Color who) -> std::string;

    // Convert a perft move to a wisdom::Move.
    auto convert_move (const wisdom::Board &board, Color who,
                       std::string move_str) -> wisdom::Move;
}

#endif // WISDOM_PERFT_HPP
