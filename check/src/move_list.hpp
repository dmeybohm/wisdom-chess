#ifndef WIZDUMB_MOVE_LIST_HPP
#define WIZDUMB_MOVE_LIST_HPP

#include <exception>
#include <vector>

#include "move.h"

class move_list_t
{
private:
    std::vector<move_t> my_moves;

public:
    move_list_t ()
    {
        my_moves.reserve(32);
    }

    move_list_t(const move_list_t &other)
    {
        my_moves = other.my_moves;
    }

    move_list_t (enum color color, std::initializer_list<const char *> list)
    {
        for (auto it : list)
        {
            my_moves.push_back(parse_move(it, color));
            color = color_invert(color);
        }
    }

    void push_back (move_t move)
    {
        my_moves.push_back (move);
    }

    void pop_back ()
    {
        my_moves.pop_back();
    }

    [[nodiscard]] auto begin() const noexcept
    {
        return my_moves.begin();
    }

    [[nodiscard]] auto end() const noexcept
    {
        return my_moves.end();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return my_moves.empty();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return my_moves.size();
    }

    [[nodiscard]] std::string to_string() const
    {
        std::string result;
        for (auto move : my_moves)
            result += ::to_string(move) + " ";
        return result;
    }

    bool operator== (const move_list_t &other) const
    {
        return my_moves.size() == other.my_moves.size () && std::equal (
                my_moves.begin(),
                my_moves.end(),
                other.my_moves.begin (),
                move_equals
                                                                 );
    }
};


#endif //WIZDUMB_MOVE_LIST_HPP
