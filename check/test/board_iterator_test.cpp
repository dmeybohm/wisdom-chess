#include "catch.hpp"
#include "board.h"

TEST_CASE( "board iteration works", "[board-iterator]")
{
    board *board = board_new();
    size_t nr_squares = 0;

    for (auto it [[maybe_unused]] : *board)
        nr_squares++;

    REQUIRE( nr_squares == 64 );
}