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
}
