#ifndef WIZDUMB_MOVE_HISTORY_HPP
#define WIZDUMB_MOVE_HISTORY_HPP

#include <string>

#include "move_list.hpp"

class MoveHistory
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

    [[nodiscard]] auto begin() const noexcept
    {
        return my_moves.begin ();
    }

    [[nodiscard]] auto end() const noexcept
    {
        return my_moves.end ();
    }

    void pop_back ()
    {
        my_moves.pop_back ();
    }

    [[nodiscard]] const MoveList& moves () const
    {
        return my_moves;
    }

    void save (const std::string &filename);
};

#endif //WIZDUMB_MOVE_HISTORY_HPP
