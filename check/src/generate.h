#ifndef EVOLVE_CHESS_BOARD_H
#define EVOLVE_CHESS_BOARD_H

#include <vector>

#include "global.h"
#include "board.h"
#include "move.h"
#include "move_tree.h"



///////////////////////////////////////////////

static inline int is_pawn_unmoved (const struct board &board,
                                   int8_t row, int8_t col)
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
const move_list_t &generate_knight_moves  (int8_t row, int8_t col);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H
