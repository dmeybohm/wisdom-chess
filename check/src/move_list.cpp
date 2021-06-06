#include "move_list.hpp"

namespace wisdom
{
    using std::vector;
    using std::unique_ptr;
    using std::make_unique;

    MoveList::MoveList (Color color, std::initializer_list<const char *> list) noexcept
    {
        for (auto it : list)
        {
            my_moves->push_back (parse_move (it, color));
            color = color_invert (color);
        }
    }

    std::string MoveList::to_string () const
    {
        std::string result = "{ ";
        for (auto move : *my_moves)
            result += "[" + wisdom::to_string (move) + "] ";
        result += "}";
        return result;
    }

    static vector<unique_ptr<MoveVector>> my_move_vector_ptrs {};

    unique_ptr<MoveVector> MoveList::allocate_move_vector ()
    {
        // todo this isn't thread safe:
        if (!my_move_vector_ptrs.empty()) {
            unique_ptr<MoveVector> ptr = std::move (my_move_vector_ptrs.back());
            my_move_vector_ptrs.pop_back();
            ptr->resize(0);
            return ptr;
        }
        return make_unique<MoveVector>();
    }

    void MoveList::deallocate_move_vector (unique_ptr<MoveVector> ptr)
    {
        my_move_vector_ptrs.push_back (std::move (ptr));
    }

    std::string to_string (const MoveList &list)
    {
        return list.to_string ();
    }

    std::ostream &operator<< (std::ostream &os, const MoveList &list)
    {
        os << to_string (list);
        return os;
    }
}
