#include "doctest/doctest.h"

// needed for working around a doctest / macOS linking problem
#include <iostream>

#include "board.h"
#include "move.h"

using namespace wisdom;

TEST_CASE( "Parsing a move" )
{
    Move with_spaces = move_parse ("   e2e4", Color::White);
    REQUIRE(make_move (coord_parse ("e2"), coord_parse ("e4")) == with_spaces );
}

TEST_CASE( "Parsing an en-passant move" )
{
    Move en_passant = move_parse ("   e4d5 ep   ", Color::White);
    Coord src = coord_parse("e4");
    Coord dst = coord_parse("d5");
    Move expected = make_en_passant_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));
    REQUIRE( en_passant == expected );
}

TEST_CASE( "Parsing a promoting move" )
{
    Move promoting = move_parse ("   d7d8 (B) ", Color::White);
    Coord src = coord_parse("d7");
    Coord dst = coord_parse("d8");
    Move expected = make_move (src, dst);
    expected = copy_move_with_promotion (expected, make_piece (Color::White, Piece::Bishop));
    REQUIRE( promoting == expected );
}

TEST_CASE( "Parsing a castling move" )
{
    Move castling = move_parse ("   o-o-o ", Color::Black);
    Coord src = coord_parse("e8");
    Coord dst = coord_parse("c8");
    Move expected = make_castling_move (ROW (src), COLUMN (src), ROW (dst), COLUMN (dst));

    REQUIRE( castling == expected );
}

TEST_CASE( "Converting a move to a string" )
{
    Move with_spaces = move_parse ("   e2e4", Color::White);
    REQUIRE( to_string(with_spaces) == std::string("e2 e4") );
    Move en_passant = move_parse ("   e4d5 ep   ", Color::White);
    REQUIRE( to_string(en_passant) == std::string("e4 d5 ep") );
    Move castling = move_parse ("   o-o-o ", Color::Black);
    REQUIRE( to_string(castling) == std::string("O-O-O") );
}

TEST_CASE( "Moving and undoing a move works" )
{
    Board board;

    Move e2e4 = move_parse ("e2e4", Color::White);
    Move d6d8 = move_parse ("d7d5", Color::Black);
    Move b1c3 = move_parse ("b1c3", Color::White);
    Move d5xe4 = move_parse ("d5xe4", Color::Black);
    Move c3xe4 = move_parse ("c3xe4", Color::White);

    UndoMove undo_state;

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