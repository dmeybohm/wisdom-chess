#include <catch/catch.hpp>

#include "board.h"
#include "move.h"

TEST_CASE( "Parsing a move", "[move]" )
{
    move_t with_spaces = move_parse ("   e2e4", Color::White);
    REQUIRE(make_move (coord_parse ("e2"), coord_parse ("e4")) == with_spaces );
}

TEST_CASE( "Parsing an en-passant move", "[move]" )
{
    move_t en_passant = move_parse ("   e4d5 ep   ", Color::White);
    coord_t src = coord_parse("e4");
    coord_t dst = coord_parse("d5");
    move_t expected = make_en_passant_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));
    REQUIRE( en_passant == expected );
}

TEST_CASE( "Parsing a promoting move", "[move]" )
{
    move_t promoting = move_parse ("   d7d8 (B) ", Color::White);
    coord_t src = coord_parse("d7");
    coord_t dst = coord_parse("d8");
    move_t expected = make_move (src, dst);
    expected = copy_move_with_promotion (expected, make_piece (Color::White, Piece::Bishop));
    REQUIRE( promoting == expected );
}

TEST_CASE( "Parsing a castling move", "[move]")
{
    move_t castling = move_parse ("   o-o-o ", Color::Black);
    coord_t src = coord_parse("e8");
    coord_t dst = coord_parse("c8");
    move_t expected = make_castling_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));

    REQUIRE( castling == expected );
}

TEST_CASE( "Converting a move to a string", "[move]" )
{
    move_t with_spaces = move_parse ("   e2e4", Color::White);
    REQUIRE( to_string(with_spaces) == "e2 e4" );
    move_t en_passant = move_parse ("   e4d5 ep   ", Color::White);
    REQUIRE( to_string(en_passant) == "e4 d5 ep" );
    move_t castling = move_parse ("   o-o-o ", Color::Black);
    REQUIRE( to_string(castling) == "O-O-O" );
}

TEST_CASE( "Moving and undoing a move works", "[move]" )
{
    struct board board;

    move_t e2e4 = move_parse ("e2e4", Color::White);
    move_t d6d8 = move_parse ("d7d5", Color::Black);
    move_t b1c3 = move_parse ("b1c3", Color::White);
    move_t d5xe4 = move_parse ("d5xe4", Color::Black);
    move_t c3xe4 = move_parse ("c3xe4", Color::White);

    undo_move_t undo_state;

    undo_state = do_move (board, Color::White, e2e4);
    undo_move (board, Color::White, e2e4, undo_state);
    do_move (board, Color::White, e2e4);

    undo_state = do_move (board, Color::Black, d6d8);
    undo_move (board, Color::Black, d6d8, undo_state);
    do_move (board, Color::Black, d6d8);

    undo_state = do_move (board, Color::White, b1c3);
    undo_move (board, Color::White, b1c3, undo_state);
    do_move (board, Color::White, b1c3);

    undo_state = do_move (board, Color::Black, d5xe4);
    undo_move (board, Color::Black, d5xe4, undo_state);
    do_move (board, Color::Black, d5xe4);

    undo_state = do_move (board, Color::White, c3xe4);
    undo_move (board, Color::White, c3xe4, undo_state);
    do_move (board, Color::White, c3xe4);
}


