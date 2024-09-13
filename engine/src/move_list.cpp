#include "move_list.hpp"

#include <iostream>

namespace wisdom
{
    auto 
    MoveList::asString() const 
        -> string
    {
        string result = "{ ";
        for (auto&& move : *this)
            result += "[" + wisdom::asString (move) + "] ";
        result += "}";
        return result;
    }

    auto 
    asString (const MoveList& list) 
        -> string
    {
        return list.asString();
    }

    auto 
    operator<< (std::ostream& os, const MoveList& list) 
        -> std::ostream&
    {
        os << asString (list);
        return os;
    }
}
