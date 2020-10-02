
#include "catch.hpp"

extern "C" {
#include "../piece.h"
#include "../board_hash.h"
#include "../board.h"
}

int Factorial( int number ) {
    //    return number <= 1 ? number : Factorial( number - 1 ) * number;  // fail
    return number <= 1 ? 1      : Factorial( number - 1 ) * number;  // pass
}

TEST_CASE( "Factorial of 0 is 1 (fail)", "[single-file]" ) {
    REQUIRE( Factorial(0) == 1 );
}

TEST_CASE( "Factorials of 1 and higher are computed (pass)", "[single-file]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}

TEST_CASE( "Board hashes can be initialized", "[single-file]" ) {
    struct board_hash board_hash;
    struct board *board = board_new();

    board_hash.hash = 0;

    board_hash_init(&board_hash, board);

    REQUIRE( board_hash.hash == 0 );
}

TEST_CASE( "Board hashes return to the same hash with a normal move", "[single-file]" ) {
    struct board_hash board_hash;
    struct board *board = board_new();

    board_hash.hash = 0;

    board_hash_init(&board_hash, board);

    // move a pawn.
    move_t mv = move_create (1, 4, 3, 4);

    uint64_t orig_board_hash = board_hash.hash;
    board_hash_move (&board_hash, board, &mv);

    REQUIRE( board_hash.hash != orig_board_hash );

    board_hash_unmove (&board_hash, board, &mv);

    REQUIRE( board_hash.hash == orig_board_hash );
}