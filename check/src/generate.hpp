#ifndef WISDOM_GENERATE_HPP
#define WISDOM_GENERATE_HPP

#include <vector>

#include "global.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "board_code.hpp"
#include "transposition_table.hpp"

namespace wisdom
{
    class MoveGenerator final
    {
    private:
        TranspositionTable &my_transposition_table;
        void sort_moves (const Board &board, MoveList &list, Color who);

    public:
        explicit MoveGenerator (TranspositionTable &transposition_table) :
            my_transposition_table { transposition_table }
        {}

        MoveList generate (const Board &board, Color who);
    };

    MoveList generate_moves (const Board &board, Color who);

    MoveList generate_legal_moves (Board &board, Color who);

    MoveList generate_captures (const Board &board, Color who);

    const MoveList &generate_knight_moves (int8_t row, int8_t col);
}

#endif // WISDOM_GENERATE_HPP
