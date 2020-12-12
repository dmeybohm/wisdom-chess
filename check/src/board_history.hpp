
#ifndef WIZDUMB_BOARD_HISTORY_HPP
#define WIZDUMB_BOARD_HISTORY_HPP

#include <unordered_map>
#include <iostream>

#include "board_code.hpp"

class board_history
{
private:
    std::unordered_map<board_code_bitset, int> position_counts;

public:
    board_history() = default;

    [[nodiscard]] int position_count (const board_code &code) const
    {
        try
        {
            return position_counts.at (code.bitset_ref ());
        }
        catch (std::out_of_range &r)
        {
            return 0;
        }
    }

    void add_board_code (const board_code &board_code)
    {
        const auto &bits = board_code.bitset_ref();
        position_counts[bits]++;
    }

    void remove_board_code (const board_code &board_code)
    {
        const auto &bits = board_code.bitset_ref();

        try
        {
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
        catch (const std::out_of_range &e)
        {
            std::cout << "Couldn't find " << board_code.to_string() << "\n";
            throw e;
        }
    }
};

#endif //WIZDUMB_BOARD_HISTORY_HPP
