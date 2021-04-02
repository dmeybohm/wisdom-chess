#ifndef WISDOM_CHESS_MOVE_TREE_HPP
#define WISDOM_CHESS_MOVE_TREE_HPP

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

        [[nodiscard]] std::string to_string () const;

        [[nodiscard]] MoveList to_list () const;

        [[nodiscard]] int size () const;
    };

}

#endif // WISDOM_CHESS_MOVE_TREE_HPP
