#include "move_tree.h"

[[nodiscard]] std::string move_tree_t::to_string () const
{
    std::string result;

    result += "{ ";
    for (auto move : list)
        result += "[" + ::to_string(move) + "] ";
    result += "}";
    return result;
}

[[nodiscard]] move_list_t move_tree_t::to_list () const
{
    move_list_t result;

    for (auto move : list)
    {
        result.push_back (move);
    }

    return result;
}

[[nodiscard]] int move_tree_t::size () const
{
    int size = 0;
    for ([[maybe_unused]] auto move : list)
        size++;
    return size;
}