#ifndef WISDOM_CHESS_MOVE_TREE_HPP
#define WISDOM_CHESS_MOVE_TREE_HPP

#include "global.hpp"
#include "move.hpp"
#include "move_list.hpp"

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

        [[nodiscard]] auto to_string () const -> string;

        [[nodiscard]] auto to_list () const -> MoveList;

        [[nodiscard]] int size () const;
    };

}

#endif // WISDOM_CHESS_MOVE_TREE_HPP
