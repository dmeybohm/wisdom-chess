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
            pushBack (moveParse (it, color));
            color = colorInvert (color);
        }
    }

    auto MoveList::asString() const -> string
    {
        string result = "{ ";
        for (auto&& move : *this)
            result += "[" + wisdom::asString (move) + "] ";
        result += "}";
        return result;
    }

    auto asString (const MoveList& list) -> string
    {
        return list.asString();
    }

    auto operator<< (std::ostream& os, const MoveList& list) -> std::ostream&
    {
        os << asString (list);
        return os;
    }
}
