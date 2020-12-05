
#include "catch.hpp"

#include "piece.h"
#include "board_hash.h"
#include "board.h"

TEST_CASE( "Board hashes can be initialized", "[single-file]" )
{
    struct board board;

    board.hash.hash = 0;

    board_hash_init (board);

    REQUIRE( board.hash.hash != 0 );
}

TEST_CASE( "Board hashes return to the same hash with a normal move", "[single-file]" )
{
    struct board board;

    board_hash_init (board);

    // move a pawn.
    move_t mv = move_parse ("e2 e4", Color::White);

    uint64_t orig_board_hash = board.hash.hash;
    undo_move_t undo_state = empty_undo_state;

    board_hash_apply_move (board, mv);

    REQUIRE( board.hash.hash != orig_board_hash );

    board_hash_unapply_move (board, mv, undo_state);

    REQUIRE( board.hash.hash == orig_board_hash );
}