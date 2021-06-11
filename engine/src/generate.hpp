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

    using ScoredMoveList = std::vector<ScoredMove>;

    class MoveGenerator final
    {
    private:
        TranspositionTable &my_transposition_table;
        ScoredMoveList to_scored_move_list (const Board &board, Color who, const MoveList &move_list);

    public:
        explicit MoveGenerator (TranspositionTable &transposition_table) :
            my_transposition_table { transposition_table }
        {}

        ScoredMoveList generate (const Board &board, Color who);
    };

    MoveList generate_moves (const Board &board, Color who);

    MoveList generate_legal_moves (Board &board, Color who);

    MoveList generate_captures (const Board &board, Color who);

    const MoveList &generate_knight_moves (int row, int col);
}

#endif // WISDOM_GENERATE_HPP
