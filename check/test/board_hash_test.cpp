
#include "catch.hpp"

extern "C"
{
#include "../src/piece.h"
#include "../src/board_hash.h"
#include "../src/board.h"
}

TEST_CASE( "Board hashes can be initialized", "[single-file]" )
{
    struct board_hash board_hash{};
    struct board *board = board_new();

    board_hash.hash = 0;

    board_hash_init(&board_hash, board);

    REQUIRE( board_hash.hash != 0 );
}

TEST_CASE( "Board hashes return to the same hash with a normal move", "[single-file]" )
{
    struct board_hash board_hash{};
    struct board *board = board_new();

    board_hash.hash = 0;

    board_hash_init (&board_hash, board);

    // move a pawn.
    move_t mv = move_create (1, 4, 3, 4);

    uint64_t orig_board_hash = board_hash.hash;
    board_hash_move (&board_hash, board, &mv);

    REQUIRE( board_hash.hash != orig_board_hash );

    board_hash_unmove (&board_hash, board, &mv);

    REQUIRE( board_hash.hash == orig_board_hash );
}