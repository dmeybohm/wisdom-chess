#ifndef EVOLVE_CHESS_MOVE_LIST_H
#define EVOLVE_CHESS_MOVE_LIST_H

#include "move.h"

typedef struct move_list
{
	struct move  *move_array;

	/* TODO: make this be ptr to end of move_array instead, so
	 * for_each_move() has a constant access to test for the end */
	int           size;
	int           len;

} move_list_t;

/* this could be optimized more because the move list length 
 * is constant across the loop */
#define for_each_move(move_ptr, moves) \
	if (likely (moves != NULL)) \
		for ((move_ptr) = &(moves)->move_array[0]; \
		     (move_ptr) != ((moves)->move_array+(moves)->len); \
		     (move_ptr)++)

move_list_t  *move_list_append  (move_list_t *move_list, 
                                 unsigned char src_row, unsigned char src_col,
                                 unsigned char dst_row, unsigned char dst_col);

/* use this for promoting moves, and when copying moves between lists */
move_list_t  *move_list_append_move (move_list_t *move_list, move_t mv);
move_list_t  *move_list_append_list (move_list_t *dst, move_list_t *src);
		
move_list_t  *move_list_sort  (move_list_t *move_list, move_t mv, int score);

void          move_list_destroy (move_list_t *move_list);

void          move_list_print   (move_list_t *move_list);

#endif // EVOLVE_CHESS_MOVE_LIST_H
