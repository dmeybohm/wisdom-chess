#ifndef WISDOM_BOARD_HISTORY_HPP
#define WISDOM_BOARD_HISTORY_HPP

#include "global.hpp"
#include "board_code.hpp"

namespace wisdom
{
    class BoardHistory
    {
    private:
        std::unordered_map<BoardCodeBitset, int> position_counts;

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
            const auto &bits = board_code.bitset_ref ();
            position_counts[bits]++;
        }

        void remove_board_code (const BoardCode& board_code) noexcept
        {
            const auto &bits = board_code.bitset_ref ();

            auto count = position_counts.at (bits) - 1;
            if (count <= 0)
            {
                position_counts.erase (bits);
            }
            else
            {
                position_counts[bits] = count;
            }
        }
    };
}

#endif //WISDOM_BOARD_HISTORY_HPP