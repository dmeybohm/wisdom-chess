#ifndef WISDOM_CHESS_VALIDATE_HPP
#define WISDOM_CHESS_VALIDATE_HPP

#include "global.hpp"
#include "move.hpp"

namespace wisdom
{
    constexpr bool validate_castling = false;

    class Board;

    void do_validate_castle_state (Board &board, Move move);

    static inline void validate_castle_state (Board &board, Move move)
    {
        if (validate_castling)
            do_validate_castle_state (board, move);
    }
}

#endif //WISDOM_CHESS_VALIDATE_HPP
