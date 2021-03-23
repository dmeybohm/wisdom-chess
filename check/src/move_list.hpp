#ifndef WIZDUMB_MOVE_LIST_HPP
#define WIZDUMB_MOVE_LIST_HPP

#include <exception>
#include <vector>

#include "move.h"

class MoveList
{
private:
    std::vector<Move> my_moves;

public:
    MoveList ()
    {
        my_moves.reserve(64);
    }

    MoveList (const MoveList &other)
    {
       my_moves = other.my_moves;
    }

	MoveList (Color color, std::initializer_list<const char*> list);

    void push_back (Move move)
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

    void append (const MoveList &other)
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

    bool operator == (const MoveList &other) const
    {
        return my_moves.size() == other.my_moves.size () && std::equal (
                my_moves.begin(),
                my_moves.end(),
                other.my_moves.begin (),
                move_equals
		);
    }

	bool operator != (const MoveList &other) const
	{
		return !(*this == other);
	}

	[[nodiscard]] const std::vector<Move> &get_my_moves () const noexcept
    {
        return my_moves;
    }

    [[nodiscard]] MoveList only_captures () const
    {
        MoveList result;
        std::copy_if (my_moves.begin(), my_moves.end(), std::back_inserter(result.my_moves), [] (Move mv) {
            return is_capture_move(mv);
        } );
        return result;
    }
};

std::string to_string (const MoveList &list);

#endif //WIZDUMB_MOVE_LIST_HPP
