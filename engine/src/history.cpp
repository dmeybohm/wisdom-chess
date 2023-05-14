#include <iostream>

#include "history.hpp"

namespace wisdom
{
    auto History::is_third_repetition (const Board& board) const -> bool
    {
        return is_nth_repetition (board, 3);
    }

    auto History::is_fifth_repetition (const Board& board) const -> bool
    {
        return is_nth_repetition (board, 5);
    }

    auto operator<< (std::ostream& os, const History& history) -> std::ostream&
    {
        int move_number = 1;
        for (const auto& code : history.my_board_codes)
            os << "Move " << move_number++ << ": " << code << "\n";
        return os;
    }
}