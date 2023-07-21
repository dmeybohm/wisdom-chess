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
        for (auto&& it : list)
        {
            push_back (moveParse (it, color));
            color = color_invert (color);
        }
    }

    auto MoveList::to_string () const -> string
    {
        string result = "{ ";
        for (auto&& move : *this)
            result += "[" + wisdom::asString (move) + "] ";
        result += "}";
        return result;
    }

    auto to_string (const MoveList& list) -> string
    {
        return list.to_string ();
    }

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&
    {
        os << to_string (list);
        return os;
    }
}
