#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "move_tree.h"

move_tree_t *tree_cache;

move_tree_t *move_tree_new (move_tree_t *parent, move_t move)
{
	move_tree_t *tree;

#if 0
	if (tree_cache)
	{
		tree = tree_cache;
		tree_cache = NULL;
	}
	else
	{
#endif
		tree = malloc (sizeof (move_tree_t));
#if 0
	}
#endif
	
	assert (tree);
	if (!tree)
		return NULL;

	tree->move   = move;
	tree->parent = parent;

	if (!parent)
		tree->depth = 0;
	else
		tree->depth = parent->depth + 1;

	return tree;
}

size_t move_tree_length (move_tree_t *tree)
{
    size_t result = 0;

    while (tree != NULL)
    {
        result++;
        tree = tree->parent;
    }

    return result;
}

// this is only safe to call if all our children are gone
void move_tree_free (move_tree_t *move_tree)
{
	assert (move_tree);

#if 0
	if (!tree_cache)
	{
		tree_cache = move_tree;

		return;
	}
#endif

	free (move_tree);
}

void move_tree_destroy (move_tree_t *move_tree)
{
	if (move_tree)
	{
		move_tree_t *parent = move_tree->parent;

		move_tree_free (move_tree);

		move_tree_destroy (parent);
	}
}

move_tree_t *move_tree_copy (move_tree_t *move_tree)
{
	move_tree_t *copy;

	if (!move_tree)
		return NULL;
	else
	{
		copy = malloc (sizeof (move_tree_t));
		
		copy->depth  = move_tree->depth;
		copy->move   = move_tree->move;
		copy->parent = move_tree_copy (move_tree->parent);

		return copy;
	}
}

/* vi: set ts=4 sw=4: */
