#include "move_tree.hpp"

namespace wisdom
{
    [[nodiscard]] auto MoveTree::to_string () const -> string
    {
        string result;

        result += "{ ";
        for (auto move : list)
            result += "[" + wisdom::to_string (move) + "] ";
        result += "}";
        return result;
    }

    [[nodiscard]] auto MoveTree::to_list () const -> MoveList
    {
        MoveList result;

        for (auto move : list)
        {
            result.push_back (move);
        }

        return result;
    }

    [[nodiscard]] auto MoveTree::size () const -> int
    {
        int size = 0;
        for ([[maybe_unused]] auto move : list)
            size++;
        return size;
    }
}
