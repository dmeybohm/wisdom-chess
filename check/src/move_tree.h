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
size_t         move_tree_length  (move_tree_t *tree);
move_tree_t   *move_tree_prepend (move_tree_t *parent, move_tree_t *child);

struct move_tree_head
{
    move_tree_t *tree;

    move_tree_head () : tree (nullptr)
    {}

    ~move_tree_head ()
    {
        move_tree_destroy (tree);
    }

    [[nodiscard]] size_t size() const
    {
        return move_tree_length (tree);
    }
};

#endif // EVOLVE_CHESS_MOVE_TREE_H
