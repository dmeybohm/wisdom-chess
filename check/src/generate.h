#ifndef EVOLVE_CHESS_BOARD_H
#define EVOLVE_CHESS_BOARD_H

#include <vector>

#include "global.h"
#include "board.h"
#include "move.h"
#include "move_tree.h"

///////////////////////////////////////////////

#define NEXT(row_or_col, direction)   ((row_or_col) + (direction))
#define VALID(row_or_col)             ((row_or_col) >= 0 && (row_or_col) < 8)
#define INVALID(row_or_col)           (!VALID (row_or_col))

///////////////////////////////////////////////

// Returns -1 if no column is eligible.
static inline int8_t eligible_en_passant_column (const struct board &board, uint8_t row, uint8_t column, enum color who)
{
    color_index_t opponent_index = color_index(color_invert(who));

    if (coord_equals (board.en_passant_target[opponent_index], no_en_passant_coord))
        return -1;

    // if WHITE rank 4, black rank 3
    if ((who == COLOR_WHITE ? 3 : 4) != row)
        return -1;

    int8_t left_column = column - 1;
    int8_t right_column = column + 1;
    uint8_t target_column = COLUMN(board.en_passant_target[opponent_index]);

    if (left_column == target_column)
    {
        assert (VALID(left_column));
        return left_column;
    }

    if (right_column == target_column)
    {
        assert (VALID(right_column));
        return right_column;
    }

    return -1;
}

static inline int is_pawn_unmoved (const struct board &board,
                                   uint8_t row, uint8_t col)
{
    assert (VALID(row) && VALID(col));
    piece_t piece = PIECE_AT (board, row, col);

    if (PIECE_COLOR(piece) == COLOR_WHITE)
        return row == 6;
    else
        return row == 1;
}

///////////////////////////////////////////////

move_list_t        generate_moves         (struct board &board, enum color who);
move_list_t        generate_legal_moves   (struct board &board, enum color who);

move_list_t        generate_captures      (struct board &board, enum color who);
const move_list_t &generate_knight_moves  (uint8_t row, uint8_t col);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H
