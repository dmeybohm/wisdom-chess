#include "move_tree.h"

namespace wisdom
{
    [[nodiscard]] std::string MoveTree::to_string () const
    {
        std::string result;

        result += "{ ";
        for (auto move : list)
            result += "[" + wisdom::to_string (move) + "] ";
        result += "}";
        return result;
    }

    [[nodiscard]] MoveList MoveTree::to_list () const
    {
        MoveList result;

        for (auto move : list)
        {
            result.push_back (move);
        }

        return result;
    }

    [[nodiscard]] int MoveTree::size () const
    {
        int size = 0;
        for ([[maybe_unused]] auto move : list)
            size++;
        return size;
    }
}