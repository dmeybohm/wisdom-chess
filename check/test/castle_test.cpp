#include "catch.hpp"

extern "C"
{
#include "../src/board.h"
}

TEST_CASE("Castling state is restored", "[castling]")
{
    struct board *board = board_new();

    move_t
}