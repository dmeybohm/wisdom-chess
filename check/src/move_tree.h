#ifndef EVOLVE_CHESS_MOVE_TREE_H
#define EVOLVE_CHESS_MOVE_TREE_H

#include "move.h"
#include "move_list.hpp"

#include <forward_list>

namespace wisdom
{
    class MoveTree
    {
    private:
        std::forward_list<Move> list;

    public:
        void push_front (Move move)
        {
            list.push_front (move);
        }

        [[nodiscard]] std::string to_string () const;

        [[nodiscard]] MoveList to_list () const;

        [[nodiscard]] int size () const;
    };
}

#endif // EVOLVE_CHESS_MOVE_TREE_H
