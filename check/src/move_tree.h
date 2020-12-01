#ifndef EVOLVE_CHESS_MOVE_TREE_H
#define EVOLVE_CHESS_MOVE_TREE_H

#include "move.h"
#include "move_list.hpp"

#include <forward_list>

class move_tree_t
{
private:
    std::forward_list<move_t> list;

public:
    void push_front (move_t move)
    {
        list.push_front (move);
    }

    [[nodiscard]] std::string to_string () const;

    [[nodiscard]] move_list_t to_list () const;

    [[nodiscard]] int size () const;
};

#endif // EVOLVE_CHESS_MOVE_TREE_H
