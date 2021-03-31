#include "move_list.hpp"

namespace wisdom
{
    MoveList::MoveList (Color color, std::initializer_list<const char *> list)
    {
        for (auto it : list)
        {
            my_moves.push_back (parse_move (it, color));
            color = color_invert (color);
        }
    }

    std::string MoveList::to_string () const
    {
        std::string result = "{ ";
        for (auto move : my_moves)
            result += "[" + wisdom::to_string (move) + "] ";
        result += "}";
        return result;
    }

    void MoveList::sort (std::function<bool(Move,Move)> compare_func)
    {
        std::sort (my_moves.begin(), my_moves.end(), std::move(compare_func));
    }

    std::string to_string (const MoveList &list)
    {
        return list.to_string ();
    }

    std::ostream &operator<< (std::ostream &os, const MoveList &list)
    {
        os << to_string(list);
        return os;
    }
}
