#include <iostream>

#include "history.hpp"

namespace wisdom
{
    auto History::isThirdRepetition (const Board& board) const -> bool
    {
        return isNthRepetition (board, 3);
    }

    auto History::isFifthRepetition (const Board& board) const -> bool
    {
        return isNthRepetition (board, 5);
    }

    auto operator<< (std::ostream& os, const History& history) -> std::ostream&
    {
        int move_number = 1;
        for (const auto& code : history.my_board_codes)
            os << "Move " << move_number++ << ": " << code << "\n";
        return os;
    }
}