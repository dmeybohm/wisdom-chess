#include <doctest/doctest.h>

// needed for working around a doctest / macOS linking problem
#include <iostream>

#include "board.hpp"
#include "board_builder.hpp"
#include "move.hpp"

using namespace wisdom;

TEST_CASE( "Parsing a move" )
{
    Move with_spaces = moveParse ("   e2e4", Color::White);
    REQUIRE( Move::make (coordParse ("e2"), coordParse ("e4")) == with_spaces );
}

TEST_CASE( "Packing source and destination coordinates" )
{
    Move a7xc8 = moveParse ("a7xc8", Color::White);
    CHECK( a7xc8.getDst () == coordParse ("c8") );
    CHECK( a7xc8.getSrc () == coordParse ("a7") );
}

TEST_CASE( "Parsing an en-passant move" )
{
    Move en_passant = moveParse ("   e4d5 ep   ", Color::White);
    Coord src = coordParse ("e4");
    Coord dst = coordParse ("d5");
    Move expected = Move::makeEnPassant (Row (src), Column (src), Row (dst), Column (dst));
    REQUIRE( en_passant == expected );
}

TEST_CASE( "Parsing a promoting move" )
{
    Move promoting = moveParse ("   d7d8 (B) ", Color::White);
    Coord src = coordParse ("d7");
    Coord dst = coordParse ("d8");
    Move expected = Move::make (src, dst);
    expected = expected.withPromotion (ColoredPiece::make (Color::White, Piece::Bishop));
    REQUIRE( promoting == expected );
}

TEST_CASE( "Parsing a castling move" )
{
    Move castling = moveParse ("   o-o-o ", Color::Black);
    Coord src = coordParse ("e8");
    Coord dst = coordParse ("c8");
    Move expected = Move::makeCastling (Row (src), Column (src), Row (dst), Column (dst));

    REQUIRE( castling == expected );
}

TEST_CASE( "Converting a move to a string" )
{
    Move with_spaces = moveParse ("   e2e4", Color::White);
    REQUIRE( asString(with_spaces) == std::string("e2 e4") );
    Move en_passant = moveParse ("   e4d5 ep   ", Color::White);
    REQUIRE( asString(en_passant) == std::string("e4 d5 ep") );
    Move castling = moveParse ("   o-o-o ", Color::Black);
    REQUIRE( asString(castling) == std::string("O-O-O") );
}

TEST_CASE("Mapping coordinates to moves")
{
    SUBCASE( "Mapping en passant" )
    {
        Board board;

        Move e2e4 = moveParse ("e2e4", Color::White);
        Move a7a5 = moveParse ("a7a5", Color::Black);
        Move e4e5 = moveParse ("e4e5", Color::White);
        Move d7d5 = moveParse ("d7d5", Color::Black);

        board = board.withMove (Color::White, e2e4);
        board = board.withMove (Color::Black, a7a5);
        board = board.withMove (Color::White, e4e5);
        board = board.withMove (Color::Black, d7d5);

        Coord e5 = coordParse ("e5");
        Coord d6 = coordParse ("d6");
        optional<Move> result = mapCoordinatesToMove (board, Color::White, e5, d6);

        Move expected = moveParse ("e5 d6 ep", Color::White);
        REQUIRE( result.has_value () );
        REQUIRE( *result == expected );
    }

    SUBCASE( "Mapping castling kingside" )
    {
        Board board;

        Move e2e4 = moveParse ("e2e4", Color::White);
        Move e7e5 = moveParse ("e7e5", Color::Black);
        Move f1c4 = moveParse ("f1c4", Color::White);
        Move d7d5 = moveParse ("f8e7", Color::Black);

        Move g1f3 = moveParse ("g1f3", Color::White);
        Move g8f6 = moveParse ("g8f6", Color::Black);

        board = board.withMove (Color::White, e2e4);
        board = board.withMove (Color::Black, e7e5);
        board = board.withMove (Color::White, f1c4);
        board = board.withMove (Color::Black, d7d5);
        board = board.withMove (Color::White, g1f3);
        board = board.withMove (Color::Black, g8f6);

        Coord e1 = coordParse ("e1");
        Coord g1 = coordParse ("g1");
        optional<Move> white_result = mapCoordinatesToMove (board, Color::White, e1, g1);

        Move white_expected = moveParse ("o-o", Color::White);
        CHECK( white_result.has_value () );
        CHECK( *white_result == white_expected );

        board = board.withMove (Color::White, white_expected);

        Coord e8 = coordParse ("e8");
        Coord g8 = coordParse ("g8");

        optional<Move> black_result = mapCoordinatesToMove (board, Color::Black, e8, g8);
        Move black_expected = moveParse ("o-o", Color::Black);

        CHECK( black_result.has_value() );
        CHECK( *black_result == black_expected );
    }

    SUBCASE( "Mapping promotion moves" )
    {
        BoardBuilder builder;

        builder.addPiece ("b7", Color::White, Piece::Pawn);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord b7 = coordParse ("b7");
        Coord b8 = coordParse ("b8");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, b7, b8, Piece::Queen);
        auto expected = moveParse ("b7b8 (Q)", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }

    SUBCASE( "Mapping promotion moves with capture" )
    {
        BoardBuilder builder;

        builder.addPiece ("c8", Color::Black, Piece::Rook);
        builder.addPiece ("b7", Color::White, Piece::Pawn);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord b7 = coordParse ("b7");
        Coord c8 = coordParse ("c8");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, b7, c8, Piece::Queen);
        auto expected = moveParse ("b7xc8 (Q)", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }

    SUBCASE( "Mapping capture moves" )
    {
        BoardBuilder builder;

        builder.addPiece ("c4", Color::Black, Piece::Pawn);
        builder.addPiece ("f1", Color::White, Piece::Bishop);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord f1 = coordParse ("f1");
        Coord c4 = coordParse ("c4");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, f1, c4);
        auto expected = moveParse ("f1xc4", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }

    SUBCASE( "Mapping normal moves" )
    {
        BoardBuilder builder;

        builder.addPiece ("c4", Color::Black, Piece::Pawn);
        builder.addPiece ("f1", Color::White, Piece::Bishop);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);

        auto board = Board { builder };

        Coord f1 = coordParse ("f1");
        Coord c3 = coordParse ("c3");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, f1, c3);
        auto expected = moveParse ("f1 c3", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }
}

TEST_CASE( "Packing and unpacking a size" )
{
    SUBCASE( "Less than 128")
    {
        size_t capacity = 72;

        Move packed_move = Move::makeAsPackedCapacity (capacity);
        size_t result = packed_move.toUnpackedCapacity();

        REQUIRE( result == capacity );
    }

    SUBCASE( "More than 128" )
    {
        size_t capacity = 240;

        Move packed_move = Move::makeAsPackedCapacity (capacity);
        size_t result = packed_move.toUnpackedCapacity();

        REQUIRE( result == capacity );
    }

    SUBCASE( "More than 32768" )
    {
        size_t capacity = 71528;

        Move packed_move = Move::makeAsPackedCapacity (capacity);
        size_t result = packed_move.toUnpackedCapacity();

        REQUIRE( result == capacity );
    }

    SUBCASE( "Serialize maximum" )
    {
        size_t capacity = Max_Packed_Capacity_In_Move;

        Move packed_move = Move::makeAsPackedCapacity (capacity);
        size_t result = packed_move.toUnpackedCapacity();

        REQUIRE( result == capacity );
    }
}
