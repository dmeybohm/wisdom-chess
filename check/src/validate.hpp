#ifndef WISDOM_VALIDATE_HPP
#define WISDOM_VALIDATE_HPP

#include "move.hpp"

namespace wisdom
{
    class Board;

    void validate_castle_state (Board &board, struct move move);
}

#endif //WISDOM_VALIDATE_HPP
