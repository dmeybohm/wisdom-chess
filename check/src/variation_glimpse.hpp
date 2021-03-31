
#ifndef WISDOM_VARIATION_GLIMPSE_HPP
#define WISDOM_VARIATION_GLIMPSE_HPP

#include <array>

#include "move.hpp"
#include "move_list.hpp"

namespace wisdom
{
    constexpr int Max_Variation_Glimpse_Size = 6;

    class VariationGlimpse // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
    private:
        std::array<Move, Max_Variation_Glimpse_Size> my_moves;
        int16_t my_start = -1;
        int16_t my_next_pos = 0;

    public:
        constexpr static int16_t next_index (int16_t index)
        {
            int result = static_cast<int> (index);
            result = (result + 1) % Max_Variation_Glimpse_Size;
            return static_cast<int16_t> (result);
        }

        void push_front (Move move)
        {
            if (my_next_pos == my_start)
                my_start = next_index (my_start);
            my_moves[my_next_pos] = move;
            if (my_start == -1)
                my_start = 0;
            my_next_pos = next_index (my_next_pos);
        }

        [[nodiscard]] std::string to_string () const;

        [[nodiscard]] MoveList to_list () const;

        [[nodiscard]] int size () const;
    };

}


#endif //WISDOM_VARIATION_GLIMPSE_HPP
