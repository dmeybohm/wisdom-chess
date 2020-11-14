#include <board_iterator.h>
#include "catch.hpp"
#include "board.h"

TEST_CASE( "board iteration works", "[board-iterator]")
{
    board *board = board_new();

    for (auto it : *board)
    {
        printf("piece: %s\n", piece_str (it));
    }
}