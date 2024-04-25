#include <iostream>

#include "history.hpp"

namespace wisdom
{
    [[nodiscard]] auto History::isProbablyThirdRepetition (const Board& board) const -> bool
    {
        return isProbablyNthRepetition (board, 3);
    }

    [[nodiscard]] auto History::isCertainlyThirdRepetition (const Board& board) const -> bool
    {
        return isCertainlyNthRepetition (board, 3);
    }

    [[nodiscard]] auto History::isProbablyFifthRepetition (const Board& board) const -> bool
    {
        return isProbablyNthRepetition (board, 5);
    }

    [[nodiscard]] auto History::isCertainlyFifthRepetition (const Board& board) const -> bool
    {
        return isCertainlyNthRepetition (board, 5);
    }

    auto History::isThirdRepetition (const Board& board) const -> bool
    {
        return isProbablyThirdRepetition (board) && isCertainlyThirdRepetition (board);
    }

    auto History::isFifthRepetition (const Board& board) const -> bool
    {
        return isProbablyFifthRepetition (board) && isCertainlyFifthRepetition (board);
    }

    auto operator<< (std::ostream& os, const History& history) -> std::ostream&
    {
        int move_number = 1;
        for (const auto& code : history.my_board_codes)
            os << "Move " << move_number++ << ": " << code << "\n";
        return os;
    }
}