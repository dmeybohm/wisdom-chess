#ifndef EVOLVE_CHESS_BOARD_H
#define EVOLVE_CHESS_BOARD_H

#include "global.h"
#include "board.h"
#include "move.h"
#include "move_list.h"
#include "move_tree.h"

///////////////////////////////////////////////

#define NEXT(row_or_col, direction)   ((row_or_col) + (direction))
#define VALID(row_or_col)             ((row_or_col) >= 0 && (row_or_col) < 8)
#define INVALID(row_or_col)           (!VALID (row_or_col))

///////////////////////////////////////////////

move_list_t    *generate_moves        (struct board *position, enum color who,
                                       move_tree_t *history);
move_list_t    *generate_legal_moves  (struct board *position, enum color who,
                                       move_tree_t *history);

move_list_t    *generate_captures     (struct board *position, enum color who,
                                       move_tree_t *history);

move_list_t    *generate_knight_moves (uint8_t row, uint8_t col);

///////////////////////////////////////////////

#endif // EVOLVE_CHESS_BOARD_H
