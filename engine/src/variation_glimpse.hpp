#ifndef WISDOM_VARIATION_GLIMPSE_HPP
#define WISDOM_VARIATION_GLIMPSE_HPP

#include "global.hpp"
#include "move.hpp"
#include "move_list.hpp"

namespace wisdom
{
    constexpr int Max_Variation_Glimpse_Size = 12;

    class VariationGlimpse // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
    private:
        array<Move, Max_Variation_Glimpse_Size> my_moves;
        int16_t my_start = -1;
        int16_t my_next_pos = 0;

    public:
        VariationGlimpse() = default;

        [[nodiscard]] constexpr static auto next_index (int16_t index) -> int16_t
        {
            int result = gsl::narrow_cast<int> (index);
            result = (result + 1) % Max_Variation_Glimpse_Size;
            return gsl::narrow_cast<int16_t> (result);
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

        [[nodiscard]] auto to_string () const -> string;

        [[nodiscard]] auto to_list () const -> MoveList;

        [[nodiscard]] auto size () const -> int;

        friend std::ostream &operator<< (std::ostream &os, const VariationGlimpse &glimpse);
    };

}

#endif //WISDOM_VARIATION_GLIMPSE_HPP
