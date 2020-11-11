#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "move_list.h"
#include "board.h"
#include "debug.h"

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

#define MAX_MOVE_LIST_CACHE_SIZE 16

static move_list_t *move_list_cache[MAX_MOVE_LIST_CACHE_SIZE];
static size_t move_list_cache_size = 0;


#define SIZE_INCREMENT    4

static move_list_t *move_list_alloc (void)
{
	move_list_t *new_moves;

	if (move_list_cache_size)
	{
	    move_list_cache_size--;
	    new_moves = move_list_cache[move_list_cache_size];
		new_moves->len = 0;
        move_list_cache[move_list_cache_size] = NULL;
		return new_moves;
	}

	new_moves = static_cast<move_list_t *>(malloc (sizeof (move_list_t)));
    assert (new_moves);
    memset (new_moves, 0, sizeof (move_list_t));

    new_moves->move_array = static_cast<move_t *>(malloc (SIZE_INCREMENT * sizeof (move_t)));
    assert (new_moves->move_array);

    new_moves->size = SIZE_INCREMENT;

	return new_moves;
}

static void move_list_dealloc (move_list_t *moves)
{
	if (moves)
	{
		if (move_list_cache_size < MAX_MOVE_LIST_CACHE_SIZE)
		{
			move_list_cache[move_list_cache_size++] = moves;
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

move_list_t *move_list_append (move_list_t *moves, uint8_t s_row,
                               uint8_t s_col, uint8_t d_row,
                               uint8_t d_col)
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
			moves->move_array = static_cast<move_t *>(realloc (moves->move_array,
                                                      moves->size * sizeof (move_t)));

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

move_list_t *move_list_append_list (move_list_t *dst, move_list_t *src)
{
	if (!src)
	    return dst;

	if (!dst)
	    dst = move_list_alloc ();

	size_t required = dst->len + src->len;
	if (dst->size < required) {
	    dst->move_array = static_cast<move_t*>(realloc (dst->move_array, required * sizeof(dst->move_array[0])));
	    assert (dst->move_array);
	    dst->size = required;
	}
	memcpy (&dst->move_array[dst->len], src->move_array, src->len * sizeof(src->move_array[0]));
    dst->len += src->len;

	return dst;
}

// vi: set ts=4 sw=4:
