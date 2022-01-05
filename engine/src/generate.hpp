#ifndef WISDOM_GENERATE_HPP
#define WISDOM_GENERATE_HPP

#include "global.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "board_code.hpp"
#include "transposition_table.hpp"

namespace wisdom
{
    struct ScoredMove
    {
        Move move;
        int score;
    };

    static_assert(std::is_trivial<ScoredMove>::value);

    class MoveGenerator final
    {
    public:
        explicit MoveGenerator (TranspositionTable &transposition_table)
        {}

        auto generate (const Board &board, Color who) -> MoveList;
    };

    auto generate_moves (const Board &board, Color who) -> MoveList;

    auto generate_legal_moves (Board &board, Color who) -> MoveList;

    auto generate_captures (const Board &board, Color who) -> MoveList;

    auto need_pawn_promotion (int row, Color who) -> bool;

    auto generate_knight_moves (int row, int col) -> const MoveList&;
}

#endif // WISDOM_GENERATE_HPP
