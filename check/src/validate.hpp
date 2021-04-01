#ifndef WISDOM_VALIDATE_HPP
#define WISDOM_VALIDATE_HPP

#include "move.hpp"

namespace wisdom
{
    constexpr bool validate_castling = true;

    void do_validate_castle_state (Board &board, Move move);

    class Board;

    static inline void validate_castle_state (Board &board, Move move)
    {
        if (validate_castling)
            do_validate_castle_state (board, move);
    }
}

#endif //WISDOM_VALIDATE_HPP
