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
        for (auto* board : history.my_previous_boards)
            os << "Move " << move_number++ << ": " << board->get_board_code() << "\n";
        return os;
    }
}