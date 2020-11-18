#ifndef WIZDUMB_MOVE_HISTORY_HPP
#define WIZDUMB_MOVE_HISTORY_HPP

#include "move_list.hpp"
#include <string>

struct move_history_t
{
private:
    move_list_t my_moves;

public:

    move_history_t() = default;

    move_history_t(const move_history_t &_other);

    void push_back (move_t move)
    {
        my_moves.push_back (move);
    }

    void pop_back ()
    {
        my_moves.pop_back ();
    }

    [[nodiscard]] move_list_t moves() const
    {
        return my_moves;
    }

    void save(std::string filename);
};

#endif //WIZDUMB_MOVE_HISTORY_HPP
