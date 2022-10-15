#include <doctest/doctest.h>

// needed for working around a doctest / macOS linking problem
#include <iostream>

#include "board.hpp"
#include "board_builder.hpp"
#include "move.hpp"

using namespace wisdom;

TEST_CASE( "Parsing a move" )
{
    Move with_spaces = move_parse ("   e2e4", Color::White);
    REQUIRE(make_regular_move (coord_parse ("e2"), coord_parse ("e4")) == with_spaces );
}

TEST_CASE( "Packing source and destination coordinates" )
{
    Move a7xc8 = move_parse ("a7xc8", Color::White);
    CHECK( move_dst (a7xc8) == coord_parse ("c8") );
    CHECK( move_src (a7xc8) == coord_parse ("a7") );
}

TEST_CASE( "Parsing an en-passant move" )
{
    Move en_passant = move_parse ("   e4d5 ep   ", Color::White);
    Coord src = coord_parse("e4");
    Coord dst = coord_parse("d5");
    Move expected = make_special_en_passant_move (Row (src), Column (src), Row (dst), Column (dst));
    REQUIRE( en_passant == expected );
}

TEST_CASE( "Parsing a promoting move" )
{
    Move promoting = move_parse ("   d7d8 (B) ", Color::White);
    Coord src = coord_parse("d7");
    Coord dst = coord_parse("d8");
    Move expected = make_regular_move (src, dst);
    expected = copy_move_with_promotion (expected, make_piece (Color::White, Piece::Bishop));
    REQUIRE( promoting == expected );
}

TEST_CASE( "Parsing a castling move" )
{
    Move castling = move_parse ("   o-o-o ", Color::Black);
    Coord src = coord_parse("e8");
    Coord dst = coord_parse("c8");
    Move expected = make_special_castling_move (Row (src), Column (src), Row (dst), Column (dst));

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

    UndoMove undo_state{};

    undo_state = board.make_move (Color::White, e2e4);
    board.take_back (Color::White, e2e4, undo_state);
    board.make_move (Color::White, e2e4);

    undo_state = board.make_move (Color::Black, d6d8);
    board.take_back (Color::Black, d6d8, undo_state);
    board.make_move (Color::Black, d6d8);

    undo_state = board.make_move (Color::White, b1c3);
    board.take_back (Color::White, b1c3, undo_state);
    board.make_move (Color::White, b1c3);

    undo_state = board.make_move (Color::Black, d5xe4);
    board.take_back (Color::Black, d5xe4, undo_state);
    board.make_move (Color::Black, d5xe4);

    undo_state = board.make_move (Color::White, c3xe4);
    board.take_back (Color::White, c3xe4, undo_state);
    board.make_move (Color::White, c3xe4);
}

TEST_CASE("Mapping coordinates to moves")
{
    SUBCASE( "Mapping en passant" )
    {
        Board board;

        Move e2e4 = move_parse ("e2e4", Color::White);
        Move a7a5 = move_parse ("a7a5", Color::Black);
        Move e4e5 = move_parse ("e4e5", Color::White);
        Move d7d5 = move_parse ("d7d5", Color::Black);

        board.make_move (Color::White, e2e4);
        board.make_move (Color::Black, a7a5);
        board.make_move (Color::White, e4e5);
        board.make_move (Color::Black, d7d5);

        Coord e5 = coord_parse ("e5");
        Coord d6 = coord_parse ("d6");
        optional<Move> result = map_coordinates_to_move (board, Color::White, e5, d6);

        Move expected = move_parse ("e5 d6 ep", Color::White);
        REQUIRE( result.has_value () );
        REQUIRE( *result == expected );
    }

    SUBCASE( "Mapping castling kingside" )
    {
        Board board;

        Move e2e4 = move_parse ("e2e4", Color::White);
        Move e7e5 = move_parse ("e7e5", Color::Black);
        Move f1c4 = move_parse ("f1c4", Color::White);
        Move d7d5 = move_parse ("f8e7", Color::Black);

        Move g1f3 = move_parse ("g1f3", Color::White);
        Move g8f6 = move_parse ("g8f6", Color::Black);

        board.make_move (Color::White, e2e4);
        board.make_move (Color::Black, e7e5);
        board.make_move (Color::White, f1c4);
        board.make_move (Color::Black, d7d5);
        board.make_move (Color::White, g1f3);
        board.make_move (Color::Black, g8f6);

        Coord e1 = coord_parse ("e1");
        Coord g1 = coord_parse ("g1");
        optional<Move> white_result = map_coordinates_to_move (board, Color::White, e1, g1);

        Move white_expected = move_parse ("o-o", Color::White);
        CHECK( white_result.has_value () );
        CHECK( *white_result == white_expected );

        board.make_move (Color::White, white_expected);

        Coord e8 = coord_parse ("e8");
        Coord g8 = coord_parse ("g8");

        optional<Move> black_result = map_coordinates_to_move (board, Color::Black, e8, g8);
        Move black_expected = move_parse ("o-o", Color::Black);

        CHECK( black_result.has_value() );
        CHECK( *black_result == black_expected );
    }

    SUBCASE( "Mapping promotion moves" )
    {
        BoardBuilder builder;

        builder.add_piece ("b7", Color::White, Piece::Pawn);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.add_piece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord b7 = coord_parse ("b7");
        Coord b8 = coord_parse ("b8");

        optional<Move> result = map_coordinates_to_move (board, Color::White, b7, b8,
                                                         Piece::Queen);
        auto expected = move_parse ("b7b8 (Q)", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }

    SUBCASE( "Mapping promotion moves with capture" )
    {
        BoardBuilder builder;

        builder.add_piece ("c8", Color::Black, Piece::Rook);
        builder.add_piece ("b7", Color::White, Piece::Pawn);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.add_piece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord b7 = coord_parse ("b7");
        Coord c8 = coord_parse ("c8");

        optional<Move> result = map_coordinates_to_move (board, Color::White, b7, c8,
                                                         Piece::Queen);
        auto expected = move_parse ("b7xc8 (Q)", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }

    SUBCASE( "Mapping capture moves" )
    {
        BoardBuilder builder;

        builder.add_piece ("c4", Color::Black, Piece::Pawn);
        builder.add_piece ("f1", Color::White, Piece::Bishop);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.add_piece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord f1 = coord_parse ("f1");
        Coord c4 = coord_parse ("c4");

        optional<Move> result = map_coordinates_to_move (board, Color::White, f1, c4);
        auto expected = move_parse ("f1xc4", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }

    SUBCASE( "Mapping normal moves" )
    {
        BoardBuilder builder;

        builder.add_piece ("c4", Color::Black, Piece::Pawn);
        builder.add_piece ("f1", Color::White, Piece::Bishop);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.add_piece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord f1 = coord_parse ("f1");
        Coord c3 = coord_parse ("c3");

        optional<Move> result = map_coordinates_to_move (board, Color::White, f1, c3);
        auto expected = move_parse ("f1 c3", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }
}

TEST_CASE( "Packing and unpacking a size" )
{
    SUBCASE( "Less than 128")
    {
        size_t capacity = 72;

        Move packed_move = make_move_with_packed_capacity (capacity);
        size_t result = unpack_capacity_from_move (packed_move);

        REQUIRE( result == capacity );
    }

    SUBCASE( "More than 128" )
    {
        size_t capacity = 240;

        Move packed_move = make_move_with_packed_capacity (capacity);
        size_t result = unpack_capacity_from_move (packed_move);

        REQUIRE( result == capacity );
    }

    SUBCASE( "More than 32768" )
    {
        size_t capacity = 71528;

        Move packed_move = make_move_with_packed_capacity (capacity);
        size_t result = unpack_capacity_from_move (packed_move);

        REQUIRE( result == capacity );
    }

    SUBCASE( "Serialize maximum" )
    {
        size_t capacity = 0x0fffFFFF;

        Move packed_move = make_move_with_packed_capacity (capacity);
        size_t result = unpack_capacity_from_move (packed_move);

        REQUIRE( result == capacity );
    }
}
