#ifndef WISDOM_MOVE_HISTORY_HPP
#define WISDOM_MOVE_HISTORY_HPP

#include "global.hpp"
#include "move_list.hpp"

namespace wisdom
{
    // todo: this just wraps the move list - could maybe just extend or remove it
    class MoveHistory final
    {
    private:
        MoveList my_moves;

    public:
        MoveHistory () = default;

        MoveHistory (const MoveHistory &_other) = default;

        explicit MoveHistory (const MoveList &list);

        void push_back (Move move)
        {
            my_moves.push_back (move);
        }

        [[nodiscard]] auto begin () const noexcept
        {
            return my_moves.begin ();
        }

        [[nodiscard]] auto end () const noexcept
        {
            return my_moves.end ();
        }

        void pop_back ()
        {
            my_moves.pop_back ();
        }

        [[nodiscard]] const MoveList &moves () const
        {
            return my_moves;
        }

        void save (const std::string &filename) const;

        std::string to_string () const;
    };
}

#endif //WISDOM_MOVE_HISTORY_HPP