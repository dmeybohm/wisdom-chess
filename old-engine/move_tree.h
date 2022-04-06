#ifndef EVOLVE_CHESS_MOVE_TREE_H
#define EVOLVE_CHESS_MOVE_TREE_H

#include "move.h"

typedef struct move_tree
{
	struct move_tree  *parent;

	move_t             move;

	int                depth;
} move_tree_t;

move_tree_t   *move_tree_new  (move_tree_t *parent, move_t move);
void           move_tree_free (move_tree_t *move_tree);

void           move_tree_destroy (move_tree_t *tree);
move_tree_t   *move_tree_copy    (move_tree_t *src);
move_tree_t   *move_tree_prepend (move_tree_t *parent, move_tree_t *child);

#endif /* EVOLVE_CHESS_MOVE_TREE_H */