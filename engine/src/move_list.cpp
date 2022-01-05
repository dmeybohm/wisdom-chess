#include "move_list.hpp"

#include <iostream>
#include <cstdlib>

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

    static vector<unique_ptr<MoveVector>> my_move_vector_ptrs {};

    auto to_string (const MoveList &list) -> string
    {
        return list.to_string ();
    }

    std::ostream &operator<< (std::ostream &os, const MoveList &list)
    {
        os << to_string (list);
        return os;
    }

    //
    // New Move list things
    //

    static constexpr std::size_t Initial_Size = 16;
    static constexpr std::size_t Size_Increment = 4;

    unique_ptr<move_list> alloc_move_list () noexcept
    {
        auto list = (move_list *) malloc (sizeof (struct move_list));
        list->move_array = (Move *) malloc (sizeof (Move) * Initial_Size);
        list->size = 0;
        list->capacity = Initial_Size;

        unique_ptr<move_list> result;
        result.reset(list);
        return result;
    }

    void dealloc_move_list (unique_ptr<move_list> move_list) noexcept
    {
        auto list = move_list.release ();
        free (list->move_array);
        free (list);
    }

    unique_ptr<move_list> copy_move_list (const move_list &from_list) noexcept
    {
        auto new_list = (move_list *) malloc (sizeof (struct move_list));
        new_list->move_array = (Move *) malloc (sizeof (Move) * from_list.size);
        memcpy (new_list->move_array, from_list.move_array, sizeof(struct Move) * from_list.size);
        new_list->size = from_list.size;
        new_list->capacity = from_list.size;

        unique_ptr<move_list> result;
        result.reset(new_list);
        return result;
    }

    std::size_t move_list_size (move_list &ptr) noexcept
    {
        return ptr.size;
    }

    std::size_t move_list_capacity (move_list &ptr) noexcept
    {
        return ptr.capacity;
    }

    Move *move_list_begin (move_list &ptr) noexcept
    {
        return ptr.move_array;
    }

    Move *move_list_end (move_list &ptr) noexcept
    {
        return ptr.move_array + ptr.size;
    }

    void move_list_append (move_list &list, Move move) noexcept
    {
        assert (list.size <= list.capacity);

        if (list.size == list.capacity)
        {
            list.capacity += Size_Increment;

            std::size_t new_size = list.capacity * sizeof (Move);
            list.move_array = (Move *) realloc (list.move_array, new_size);
        }

        assert (list.move_array != nullptr);
        list.move_array[list.size++] = move;
    }

    void move_list_pop (move_list &list) noexcept
    {
        assert (list.size > 0);
        list.size--;
    }
}
