#ifndef WIZDUMB_MOVE_HISTORY_HPP
#define WIZDUMB_MOVE_HISTORY_HPP

#include <string>

#include "move_list.hpp"

struct move_history_t
{
private:
    move_list_t my_moves;

public:
    move_history_t () = default;

    move_history_t (const move_history_t &_other);

    move_history_t (const move_list_t &list);

    void push_back (move_t move)
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

    [[nodiscard]] const move_list_t& moves() const
    {
        return my_moves;
    }

    void save (const std::string &filename);
};

#endif //WIZDUMB_MOVE_HISTORY_HPP
