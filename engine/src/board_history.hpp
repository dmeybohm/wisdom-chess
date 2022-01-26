#ifndef WISDOM_BOARD_HISTORY_HPP
#define WISDOM_BOARD_HISTORY_HPP

#include "global.hpp"
#include "board_code.hpp"

namespace wisdom
{
    class BoardHistory
    {
    private:
        phmap::parallel_flat_hash_map<BoardCodeBitset, int> position_counts;

    public:
        BoardHistory () = default;

        [[nodiscard]] auto position_count (const BoardCode& code) const noexcept
            -> int
        {
            auto iterator = position_counts.find (code.bitset_ref ());
            return iterator == position_counts.end () ? 0 : iterator->second;
        }

        void add_board_code (const BoardCode& board_code) noexcept
        {
            const auto& bits = board_code.bitset_ref ();
            position_counts[bits]++;
        }

        void remove_board_code (const BoardCode& board_code) noexcept
        {
            const auto& bits = board_code.bitset_ref ();
            auto iterator = position_counts.find (bits);
            assert (iterator != position_counts.end ());

            auto count = iterator->second - 1;
            if (count <= 0)
            {
                position_counts.erase (iterator);
            }
            else
            {
                iterator->second = count;
            }
        }
    };
}

#endif //WISDOM_BOARD_HISTORY_HPP
