#include "catch.hpp"

#include "board.h"
#include "move.h"

TEST_CASE( "Parsing a move", "[move]" )
{
    move_t with_spaces = move_parse ("   e2e4", COLOR_WHITE);
    REQUIRE( move_create (coord_parse("e2"), coord_parse("e4")) == with_spaces );
}

TEST_CASE( "Parsing an en-passant move", "[move]" )
{
    move_t en_passant = move_parse ("   e4d5 ep   ", COLOR_WHITE);
    coord_t src = coord_parse("e4");
    coord_t dst = coord_parse("d5");
    move_t expected = move_create_en_passant (ROW(src), COLUMN(src), ROW(dst), COLUMN(dst));
    REQUIRE( en_passant == expected );
}

TEST_CASE( "Parsing a promoting move", "[move]" )
{
    move_t promoting = move_parse ("   d7d8 (B) ", COLOR_WHITE);
    coord_t src = coord_parse("d7");
    coord_t dst = coord_parse("d8");
    move_t expected = move_create (src, dst);
    expected = move_with_promotion (expected, MAKE_PIECE (COLOR_WHITE, PIECE_BISHOP));
    REQUIRE( promoting == expected );
}

TEST_CASE( "Parsing a castling move", "[move]")
{
    move_t castling = move_parse ("   o-o-o ", COLOR_BLACK);
    coord_t src = coord_parse("e8");
    coord_t dst = coord_parse("c8");
    move_t expected = move_create_castling (ROW(src), COLUMN(src), ROW(dst), COLUMN(dst));

    REQUIRE( castling == expected );
}

TEST_CASE( "Moving and undoing a move works", "[move]" )
{
    struct board board;

    move_t e2e4 = move_parse ("e2e4", COLOR_WHITE);
    move_t d6d8 = move_parse ("d7d5", COLOR_BLACK);
    move_t b1c3 = move_parse ("b1c3", COLOR_WHITE);
    move_t d5xe4 = move_parse ("d5xe4", COLOR_BLACK);
    move_t c3xe4 = move_parse ("c3xe4", COLOR_WHITE);

    undo_move_t undo_state;

    undo_state = do_move (board, COLOR_WHITE, e2e4);
    undo_move (board, COLOR_WHITE, e2e4, undo_state);
    do_move (board, COLOR_WHITE, e2e4);

    undo_state = do_move (board, COLOR_BLACK, d6d8);
    undo_move (board, COLOR_BLACK, d6d8, undo_state);
    do_move (board, COLOR_BLACK, d6d8);

    undo_state = do_move (board, COLOR_WHITE, b1c3);
    undo_move (board, COLOR_WHITE, b1c3, undo_state);
    do_move (board, COLOR_WHITE, b1c3);

    undo_state = do_move (board, COLOR_BLACK, d5xe4);
    undo_move (board, COLOR_BLACK, d5xe4, undo_state);
    do_move (board, COLOR_BLACK, d5xe4);

    undo_state = do_move (board, COLOR_WHITE, c3xe4);
    undo_move (board, COLOR_WHITE, c3xe4, undo_state);
    do_move (board, COLOR_WHITE, c3xe4);
}


