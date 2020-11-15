#include "catch.hpp"
#include "board.h"

TEST_CASE( "board iteration works", "[board-iterator]")
{
    struct board board;
    size_t nr_squares = 0;

    for (auto it [[maybe_unused]] : board)
        nr_squares++;

    REQUIRE( nr_squares == 64 );
}