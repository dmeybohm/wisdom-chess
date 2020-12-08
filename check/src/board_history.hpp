
#ifndef WIZDUMB_BOARD_HISTORY_HPP
#define WIZDUMB_BOARD_HISTORY_HPP

#include <unordered_map>

#include "board_code.hpp"

class board_history
{
private:
    std::unordered_map<board_code_bitset, int> position_counts;
    int half_move_count{};

public:
    board_history() = default;

    [[nodiscard]] int total_half_moves () const
    {
        return half_move_count;
    }

    [[nodiscard]] int total_full_moves () const
    {
        return half_move_count / 2;
    }

    void add_board_code (const board_code &board_code);
    void remove_board_code (const board_code &board_code);
};

#endif //WIZDUMB_BOARD_HISTORY_HPP
