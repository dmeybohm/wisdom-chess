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
static inline int8_t eligible_en_passant_column (const struct board *board, uint8_t row, uint8_t column, enum color who)
{
    color_index_t opponent_index = color_index(color_invert(who));

    // if WHITE rank 4, black rank 3
    if ((who == COLOR_WHITE ? 3 : 4) != row)
        return -1;

    if (column == board->en_passant_columns[opponent_index][0])
    {
        // *this* pawn is to the left of the taken pawn, so the take column is +1
        int8_t result = column + 1;
        assert (VALID(result));
        return result;
    }

    if (column == board->en_passant_columns[opponent_index][1])
    {
        int8_t result = column - 1;
        assert (VALID(result));
        return result;
    }

    return -1;
}

///////////////////////////////////////////////


move_list_t generate_moves        (struct board *board, enum color who);
move_list_t generate_legal_moves (struct board *board, enum color who);

move_list_t generate_captures (struct board *board, enum color who);
move_list_t generate_knight_moves (uint8_t row, uint8_t col);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H
