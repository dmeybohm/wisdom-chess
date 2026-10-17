#include <algorithm>
#include <iostream>

#include "wisdom-chess/engine/move_list.hpp"

namespace wisdom
{
    auto
    OrderedMoveList::next() noexcept
        -> optional<Move>
    {
        if (my_next == my_size)
            return nullopt;

        auto rest = my_moves.begin() + narrow_debug<std::ptrdiff_t> (my_next);
        auto end = my_moves.begin() + narrow_debug<std::ptrdiff_t> (my_size);
        auto by_key = [] (const KeyedMove& a, const KeyedMove& b) { return a.key < b.key; };

        if (my_next == 0)
            std::iter_swap (rest, std::min_element (rest, end, by_key));
        else if (my_next == 1)
            std::sort (rest, end, by_key);

        return my_moves[my_next++].move;
    }

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
