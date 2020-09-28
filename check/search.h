#ifndef EVOLVE_CHESS_SEARCH_H_
#define EVOLVE_CHESS_SEARCH_H_

#include "board.h"
#include "piece.h"
#include "move.h"
#include "move_tree.h"

move_t  find_best_move   (struct board *board, color_t side, 
                          move_tree_t *history);

int is_overdue ();

#define INFINITY  	65536

#endif /* EVOLVE_CHESS_SEARCH_H_ */
