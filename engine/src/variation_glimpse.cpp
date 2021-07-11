#include "variation_glimpse.hpp"

#include <sstream>

namespace wisdom
{
    [[nodiscard]] std::string VariationGlimpse::to_string () const
    {
        MoveList result = to_list ();

        return result.to_string ();
    }

    [[nodiscard]] MoveList VariationGlimpse::to_list () const
    {
        std::list<Move> moves_tmp_list;

        if (my_start == -1)
            return MoveList {};

        Move move = my_moves[my_start];
        for (int16_t pos = next_index (my_start);
            pos != my_next_pos;
            pos = next_index (pos))
        {
            moves_tmp_list.push_front (move);
            move = my_moves[pos];
        }
        moves_tmp_list.push_front (move);

        MoveList result;
        for (auto move : moves_tmp_list)
            result.push_back (move);

        return result;
    }

    [[nodiscard]] int VariationGlimpse::size () const
    {
        if (my_start == -1)
            return 0;
        if (my_start == my_next_pos)
            return Max_Variation_Glimpse_Size;
        return my_next_pos;
    }

    std::ostream &operator<< (std::ostream &os, const VariationGlimpse &glimpse)
    {
        MoveList tmp_list = glimpse.to_list ();

        os << "{ my_moves: " << wisdom::to_string (tmp_list) << " my_start: " << glimpse.my_start << " my_next_pos: "
           << glimpse.my_next_pos << " }";
        return os;
    }

}