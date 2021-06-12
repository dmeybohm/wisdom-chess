#include <doctest/doctest.h>
#include "board.hpp"
#include "board_builder.hpp"

#include <algorithm>
#include <iostream>

#include "board_code.hpp"

using namespace wisdom;

TEST_CASE( "Board code is able to be set" )
{
    BoardCode code, initial;

    auto initial_str = code.to_string ();
    std::size_t num_zeroes = std::count (initial_str.begin(), initial_str.end(), '0');
    REQUIRE( num_zeroes == initial_str.size () );

    Coord a8 = coord_parse ("a8");
    ColoredPiece black_pawn = make_piece (Color::Black, Piece::Pawn);
    code.add_piece (a8, black_pawn);

    REQUIRE( code != initial );

    code.remove_piece (a8);

    REQUIRE( code == initial );

    Coord h1 = coord_parse ("h1");
    ColoredPiece white_king = make_piece (Color::White, Piece::King);
    code.add_piece (h1, white_king);

    std::string result = code.to_string();
    result = result.substr (0, 4);
    REQUIRE( result == "0001" );

    code.remove_piece (h1);
    result = code.to_string().substr(0, 4);
    REQUIRE( result == "0000" );
}

TEST_CASE( "Capturing moves are applied and undone correctly" )
{
    BoardBuilder builder;

    builder.add_piece ("a8", Color::White, Piece::Bishop);
    builder.add_piece ("b7", Color::Black, Piece::Knight);
    builder.add_piece ("e1", Color::Black, Piece::King);
    builder.add_piece ("e8", Color::White, Piece::King);

    Board brd = builder.build();
    BoardCode code { brd };
    BoardCode initial = code;

    REQUIRE( initial.count_ones() > 0 );

    Move a8xb7 = parse_move ("a8xb7");
    code.apply_move (brd, a8xb7);
    REQUIRE( initial != code );

    UndoMove undo_state = do_move (brd, Color::White, a8xb7);
    code.unapply_move (brd, a8xb7, undo_state);
    REQUIRE( initial == code );
}

TEST_CASE( "Promoting moves are applied and undone correctly" )
{
    BoardBuilder builder;

    builder.add_piece ("a8", Color::White, Piece::Bishop);
    builder.add_piece ("b7", Color::Black, Piece::Pawn);
    builder.add_piece ("e1", Color::Black, Piece::King);
    builder.add_piece ("e8", Color::White, Piece::King);

    Board brd = builder.build();
    BoardCode code { brd };
    BoardCode initial = code;

    REQUIRE( initial.count_ones() > 0 );

    Move b7b8_Q = parse_move ("b7b8_Q (Q)");
    code.apply_move (brd, b7b8_Q);
    REQUIRE( initial != code );

    UndoMove undo_state = do_move (brd, Color::Black, b7b8_Q);
    code.unapply_move (brd, b7b8_Q, undo_state);
    REQUIRE( initial == code );
}

TEST_CASE( "Castling moves are applied and undone correctly" )
{
    BoardBuilder builder;

    builder.add_piece ("a8", Color::Black, Piece::Rook);
    builder.add_piece ("b6", Color::Black, Piece::Pawn);
    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    Board brd = builder.build();
    BoardCode code {brd };
    BoardCode initial = code;

    REQUIRE( initial.count_ones() > 0 );

    Move castle_queenside = parse_move ("o-o-o", Color::Black);
    code.apply_move (brd, castle_queenside);
    REQUIRE( initial != code );

    UndoMove undo_state = do_move (brd, Color::Black, castle_queenside);
    code.unapply_move (brd, castle_queenside, undo_state);
    REQUIRE( initial == code );
}

TEST_CASE( "Promoting+Capturing moves are applied and undone correctly" )
{
    BoardBuilder builder;

    builder.add_piece ("a8", Color::White, Piece::Rook);
    builder.add_piece ("b7", Color::Black, Piece::Pawn);
    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    Board brd = builder.build();
    BoardCode code {brd };
    BoardCode initial = code;

    REQUIRE( initial.count_ones() > 0 );

    Move promote_castle_move = parse_move ("b7xa8 (Q)", Color::Black);
    REQUIRE( is_promoting_move(promote_castle_move) );
    REQUIRE( is_capture_move(promote_castle_move) );

    code.apply_move (brd, promote_castle_move);
    REQUIRE( initial != code );

    UndoMove undo_state = do_move (brd, Color::Black, promote_castle_move);
    code.unapply_move (brd, promote_castle_move, undo_state);
    REQUIRE( initial == code );
}