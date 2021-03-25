#ifndef WISDOM_GENERATE_HPP
#define WISDOM_GENERATE_HPP

#include <vector>

#include "global.hpp"
#include "board.hpp"
#include "move.hpp"
#include "move_tree.hpp"
#include "board_code.hpp"
#include "transposition_table.hpp"

namespace wisdom
{
    class MoveGenerator final
    {
    private:
        TranspositionTable my_transposition_table;

    public:
        MoveList sort_moves (MoveList &list);
    };

    static inline int is_pawn_unmoved (const struct Board &board,
                                       int8_t row, int8_t col)
    {
        assert (is_valid_row (row) && is_valid_column (col));
        ColoredPiece piece = piece_at (board, row, col);

        if (piece_color (piece) == Color::White)
            return row == 6;
        else
            return row == 1;
    }

    MoveList generate_moves (const Board &board, Color who);

    MoveList generate_legal_moves (Board &board, Color who);

    MoveList generate_captures (const Board &board, Color who);

    const MoveList &generate_knight_moves (int8_t row, int8_t col);
}

#endif // WISDOM_GENERATE_HPP
