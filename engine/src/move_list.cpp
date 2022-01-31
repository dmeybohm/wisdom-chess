#include "move_list.hpp"

#include <cstdlib>
#include <iostream>

namespace wisdom
{
    MoveList::MoveList (Color color, std::initializer_list<czstring> list) noexcept
    {
        for (auto it : list)
        {
            push_back (move_parse (it, color));
            color = color_invert (color);
        }
    }

    auto MoveList::to_string () const -> string
    {
        string result = "{ ";
        for (auto move : *this)
            result += "[" + wisdom::to_string (move) + "] ";
        result += "}";
        return result;
    }

    auto to_string (const MoveList& list) -> string { return list.to_string (); }

    std::ostream& operator<< (std::ostream& os, const MoveList& list)
    {
        os << to_string (list);
        return os;
    }

    //
    // New Move list things
    //

    static constexpr std::size_t Initial_Size = 16;
    static constexpr std::size_t Size_Increment = 4;
    static vector<unique_ptr<move_list>> move_list_ptrs;

    unique_ptr<move_list> alloc_move_list () noexcept
    {
        if (!move_list_ptrs.empty ())
        {
            auto move_list_end = std::move (move_list_ptrs.back ());
            move_list_ptrs.pop_back ();
            return move_list_end;
        }

        auto list = (move_list*)malloc (sizeof (struct move_list));
        list->move_array = (Move*)malloc (sizeof (Move) * Initial_Size);
        list->capacity = Initial_Size;

        unique_ptr<move_list> result;
        result.reset (list);
        return result;
    }

    void dealloc_move_list (unique_ptr<move_list> move_list) noexcept
    {
        move_list_ptrs.emplace_back (std::move (move_list));
    }

    std::size_t move_list_capacity (move_list& ptr) noexcept { return ptr.capacity; }

    void move_list_append (move_list& list, std::size_t position, Move move) noexcept
    {
        assert (position <= list.capacity);

        if (position == list.capacity)
        {
            list.capacity += Size_Increment;

            std::size_t new_capacity = list.capacity * sizeof (Move);
            list.move_array = (Move*)realloc (list.move_array, new_capacity);
        }

        assert (list.move_array != nullptr);
        list.move_array[position] = move;
    }
}
