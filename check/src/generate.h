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
    assert (is_valid_row(row) && is_valid_column(col));
    piece_t piece = piece_at (board, row, col);

    if (piece_color(piece) == Color::White)
        return row == 6;
    else
        return row == 1;
}

///////////////////////////////////////////////

move_list_t        generate_moves         (struct board &board, Color who);
move_list_t        generate_legal_moves   (struct board &board, Color who);

move_list_t        generate_captures      (struct board &board, Color who);
const move_list_t &generate_knight_moves  (int8_t row, int8_t col);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H
