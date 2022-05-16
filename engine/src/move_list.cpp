#include "move_list.hpp"

#include <cstdlib>
#include <iostream>

namespace wisdom
{
    // When using an initalizer list, that is used in a context where performance
    // isn't that important. So use the default, private, uncached constructor.
    MoveList::MoveList (Color color, std::initializer_list<czstring> list) noexcept
        : MoveList {}
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

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&
    {
        os << to_string (list);
        return os;
    }

    //
    // New Move list things
    //

    std::size_t move_list_capacity (move_list& ptr) noexcept { return ptr.capacity; }

    void move_list_append (move_list& list, std::size_t position, Move move) noexcept
    {
        assert (position <= list.capacity);

        if (position == list.capacity)
        {
            list.capacity += MoveListAllocator::Size_Increment;

            std::size_t new_capacity_in_bytes = list.capacity * sizeof (Move);
            list.move_array = (Move*)realloc (list.move_array, new_capacity_in_bytes);
        }

        assert (list.move_array != nullptr);
        list.move_array[position] = move;
    }

    auto MoveListAllocator::alloc_move_list() noexcept -> unique_ptr<move_list>
    {
        if (!my_move_list_ptrs.empty ())
        {
            auto move_list_end = std::move (my_move_list_ptrs.back ());
            my_move_list_ptrs.pop_back ();
            return move_list_end;
        }

        return default_alloc_move_list ();
    }

    void MoveListAllocator::dealloc_move_list (unique_ptr<move_list> move_list) noexcept
    {
        if (move_list != nullptr)
            my_move_list_ptrs.emplace_back (std::move (move_list));
    }
}
