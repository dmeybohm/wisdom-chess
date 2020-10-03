#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "move_list.h"
#include "board.h"
#include "debug.h"

DEFINE_DEBUG_CHANNEL(move_list, 0);

/*
 * TODO: implement a cache for allocating.  This would have an
 * extremley high hit rate:
 *
 *                      *                 [nodes]
 *                    /   \
 *                   D     F
 *                 /   \
 *                C     E
 *              /  \
 *             A    B
 *
 *  When node A is done with the move list, node C calls search()
 *  on node B.  Node B will then allocate what used to be the move
 *  list on node A.  Similarly, when B is done, C will deallocate
 *  its move list and then node E will get that one.  
 *
 *  So, the cache would only need to be as big as the depth of the
 *  move tree, and hence very small (maybe 4-8 items -- I think 4
 *  would be one cacheline on Pentium, would cover most
 *  possible depths).  This should reduce the penalty for dynamic
 *  allocation to almost nothing.  
 *
 *  You could allocate the tree on the stack, but that seems
 *  messier to me.
 */

static move_list_t *move_list_cache;

#define SIZE_INCREMENT    4

static move_list_t *move_list_alloc (void)
{
	move_list_t *new_moves;

	if (move_list_cache)
	{
		move_list_cache->len = 0;

		new_moves = move_list_cache;
		move_list_cache = NULL;

		return new_moves;
	}

	new_moves = malloc (sizeof (move_list_t));

	if (new_moves)
	{
		memset (new_moves, 0, sizeof (move_list_t));

		new_moves->move_array = malloc (SIZE_INCREMENT * sizeof (move_t));

		if (!new_moves->move_array)
		{
			free (new_moves);
			return NULL;
		}

		new_moves->size = SIZE_INCREMENT;
	}
	
	return new_moves;
}

static void move_list_dealloc (move_list_t *moves)
{
	if (moves)
	{
		if (!move_list_cache)
		{
			move_list_cache = moves;
			return;
		}

		if (moves->move_array)
			free (moves->move_array);

		free (moves);
	}
}

void move_list_destroy (move_list_t *moves)
{
	move_list_dealloc (moves);
}

move_list_t *move_list_append (move_list_t *moves, unsigned char s_row, 
                               unsigned char s_col, unsigned char d_row, 
                               unsigned char d_col)
{
	move_t  move;

	move = move_create (s_row, s_col, d_row, d_col);

	return move_list_append_move (moves, move);
}

move_list_t *move_list_append_move (move_list_t *moves, move_t move)
{
	if (!moves)
		moves = move_list_alloc ();

	if (moves)
	{
		assert (moves->len <= moves->size);

		if (moves->len == moves->size)
		{
			moves->size += SIZE_INCREMENT;
			moves->move_array = realloc (moves->move_array, 
			                             moves->size * sizeof (move_t));

			if (!moves->move_array)
			{
				free (moves);
				return NULL;
			}
		}

		moves->move_array[moves->len++] = move;
	}
	
	return moves;
}

char row_to_char (int row)
{
	    return 8-row + '0';
}

char col_to_char (int col)
{
	    return col + 'a';
}

void move_list_print (move_list_t *move_list)
{
	move_t    *mv;
	
	return;

	debug_multi_line_start (&CHANNEL_NAME (move_list));

	DBG (move_list, "move_list: { ");

	for_each_move (mv, move_list)
	{
		coord_t src, dst;

		src = MOVE_SRC (*mv);
		dst = MOVE_DST (*mv);

		DBG (move_list, "[%c%c %c%c] ", col_to_char (COLUMN (src)), 
		                                row_to_char (ROW (src)),
		                                col_to_char (COLUMN (dst)),
		                                row_to_char (ROW (dst)));
	}

	DBG (move_list, "}\n");

	debug_multi_line_stop (&CHANNEL_NAME (move_list));
}

/* TODO: optmize this with memcpy */
move_list_t *move_list_append_list (move_list_t *dst, move_list_t *src)
{
	move_t *mv;

	for_each_move (mv, src)
		dst = move_list_append_move (dst, *mv);

	return dst;
}

/* vi: set ts=4 sw=4: */
