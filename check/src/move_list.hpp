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
        my_moves.reserve(64);
    }

    move_list_t (const move_list_t &other)
    {
       my_moves = other.my_moves;
    }

	move_list_t (Color color, std::initializer_list<const char*> list);

    void push_back (move_t move)
    {
        my_moves.push_back (move);
    }

    void pop_back ()
    {
        my_moves.pop_back();
    }

    [[nodiscard]] auto begin () const noexcept
    {
        return my_moves.begin();
    }

    [[nodiscard]] auto end () const noexcept
    {
        return my_moves.end();
    }

    void append (const move_list_t &other)
    {
        my_moves.insert (my_moves.end(), other.begin(), other.end());
    }

    [[nodiscard]] bool empty () const noexcept
    {
        return my_moves.empty();
    }

    [[nodiscard]] size_t size () const noexcept
    {
        return my_moves.size();
    }

	[[nodiscard]] std::string to_string () const;

    bool operator == (const move_list_t &other) const
    {
        return my_moves.size() == other.my_moves.size () && std::equal (
                my_moves.begin(),
                my_moves.end(),
                other.my_moves.begin (),
                move_equals
		);
    }

	bool operator != (const move_list_t &other) const
	{
		return !(*this == other);
	}

	[[nodiscard]] const std::vector<move_t> &get_my_moves () const noexcept
    {
        return my_moves;
    }
};

std::string to_string (const move_list_t &list);

#endif //WIZDUMB_MOVE_LIST_HPP
