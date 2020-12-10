
#ifndef WIZDUMB_BOARD_HISTORY_HPP
#define WIZDUMB_BOARD_HISTORY_HPP

#include <unordered_map>

#include "board_code.hpp"

class board_history
{
private:
    std::unordered_map<board_code_bitset, int> position_counts;

public:
    board_history() = default;

    [[nodiscard]] int position_count (const board_code &code) const
    {
        return position_counts.at (code.bitset_ref());
    }

    void add_board_code (const board_code &board_code)
    {
        const auto &bits = board_code.bitset_ref();
        position_counts[bits]++;
    }

    void remove_board_code (const board_code &board_code)
    {
        const auto &bits = board_code.bitset_ref();

        auto count = position_counts.at(bits) - 1;
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

#endif //WIZDUMB_BOARD_HISTORY_HPP
