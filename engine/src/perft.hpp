#ifndef WISDOM_PERFT_HPP
#define WISDOM_PERFT_HPP

#include "global.hpp"

namespace wisdom
{
    class Board;
    class MoveList;
    enum class Color;
}

namespace wisdom::perft
{
    struct MoveCounter
    {
        int nodes = 0;
        int captures = 0;
        int en_passants = 0;
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
    };

    auto to_move_list (const wisdom::Board &board, Color who,
                       const std::string &move_list) -> wisdom::MoveList;
}

#endif // WISDOM_PERFT_HPP
