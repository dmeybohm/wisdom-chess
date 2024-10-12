#include <iostream>

#include "wisdom-chess/engine/history.hpp"

namespace wisdom
{
    bool History::isProbablyThirdRepetition (const Board& board) const
    {
        return isProbablyNthRepetition (board, 3);
    }

    bool History::isCertainlyThirdRepetition (const Board& board) const
    {
        return isCertainlyNthRepetition (board, 3);
    }

    bool History::isProbablyFifthRepetition (const Board& board) const
    {
        return isProbablyNthRepetition (board, 5);
    }

    bool History::isCertainlyFifthRepetition (const Board& board) const
    {
        return isCertainlyNthRepetition (board, 5);
    }

    bool History::isThirdRepetition (const Board& board) const
    {
        return isProbablyThirdRepetition (board) && isCertainlyThirdRepetition (board);
    }

    bool History::isFifthRepetition (const Board& board) const
    {
        return isProbablyFifthRepetition (board) && isCertainlyFifthRepetition (board);
    }

    auto 
    operator<< (std::ostream& os, const History& history) 
        -> std::ostream&
    {
        int move_number = 1;
        for (const auto& code : history.my_board_codes)
            os << "Move " << move_number++ << ": " << code << "\n";
        return os;
    }
}
