
#include "board_history.hpp"

void board_history::add_board_code (const board_code &board_code)
{
    const auto &bits = board_code.bitset_ref();
    position_counts[bits]++;
    half_move_count++;
}

void board_history::remove_board_code (const board_code &board_code)
{
    const auto &bits = board_code.bitset_ref();

    auto count = position_counts[bits] - 1;
    if (count <= 0)
    {
        position_counts.erase (bits);
    }
    else
    {
        position_counts[bits] = count;
    }

    half_move_count--;
}
