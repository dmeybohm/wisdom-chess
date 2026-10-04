// needed for working around a doctest / macOS linking problem
#include <iostream>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/move.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "Parsing a move" )
{
    Move with_spaces = toMove ("   e2e4", Color::White);
    REQUIRE( Move::make (toCoord ("e2"), toCoord ("e4")) == with_spaces );
}

TEST_CASE( "Packing source and destination coordinates" )
{
    Move a7xc8 = toMove ("a7xc8", Color::White);
    CHECK( a7xc8.getDst() == toCoord ("c8") );
    CHECK( a7xc8.getSrc() == toCoord ("a7") );
}

TEST_CASE( "Parsing an en-passant move" )
{
    Move en_passant = toMove ("   e4d5 ep   ", Color::White);
    Coord src = toCoord ("e4");
    Coord dst = toCoord ("d5");
    Move expected = Move::makeEnPassant (src.row(), src.column(), dst.row(), dst.column());
    REQUIRE( en_passant == expected );
}

TEST_CASE( "Only a castling move needs a color to parse" )
{
    CHECK( toMove ("e5 d6 ep") == toMove ("e5 d6 ep", Color::White) );
    CHECK( toMove ("d7 d8 (Q)") == toMove ("d7 d8 (Q)", Color::White) );
    CHECK( toMove ("e2 d1 (N)") == toMove ("e2 d1 (N)", Color::Black) );
    CHECK_FALSE( parseMove ("o-o", Color::None).has_value() );
}

TEST_CASE( "Parsing a promoting move" )
{
    Move promoting = toMove ("   d7d8 (B) ", Color::White);
    Coord src = toCoord ("d7");
    Coord dst = toCoord ("d8");
    Move expected = Move::make (src, dst);
    expected = expected.withPromotion (Piece::Bishop);
    REQUIRE( promoting == expected );
}

TEST_CASE( "Parsing a castling move" )
{
    Move castling = toMove ("   o-o-o ", Color::Black);
    Coord src = toCoord ("e8");
    Coord dst = toCoord ("c8");
    Move expected = Move::makeCastling (src.row(), src.column(), dst.row(), dst.column());

    REQUIRE( castling == expected );
}

TEST_CASE( "Converting a move to a string" )
{
    Move with_spaces = toMove ("   e2e4", Color::White);
    REQUIRE( asString (with_spaces) == std::string {"e2 e4" } );

    Move en_passant = toMove ("   e4d5 ep   ", Color::White);
    REQUIRE( asString (en_passant) == std::string {"e4 d5 ep" } );

    Move castling = toMove ("   o-o-o ", Color::Black);
    REQUIRE( asString (castling) == std::string { "O-O-O" } );
}

TEST_CASE( "Mapping coordinates to moves" )
{
    SUBCASE( "Mapping en passant" )
    {
        Board board;

        Move e2e4 = toMove ("e2e4", Color::White);
        Move a7a5 = toMove ("a7a5", Color::Black);
        Move e4e5 = toMove ("e4e5", Color::White);
        Move d7d5 = toMove ("d7d5", Color::Black);

        board = board.withMove (Color::White, e2e4);
        board = board.withMove (Color::Black, a7a5);
        board = board.withMove (Color::White, e4e5);
        board = board.withMove (Color::Black, d7d5);

        Coord e5 = toCoord ("e5");
        Coord d6 = toCoord ("d6");
        optional<Move> result = mapCoordinatesToMove (board, Color::White, e5, d6);

        Move expected = toMove ("e5 d6 ep", Color::White);
        REQUIRE( result.has_value() );
        REQUIRE( *result == expected );
    }

    SUBCASE( "Mapping castling kingside" )
    {
        Board board;

        Move e2e4 = toMove ("e2e4", Color::White);
        Move e7e5 = toMove ("e7e5", Color::Black);
        Move f1c4 = toMove ("f1c4", Color::White);
        Move d7d5 = toMove ("f8e7", Color::Black);

        Move g1f3 = toMove ("g1f3", Color::White);
        Move g8f6 = toMove ("g8f6", Color::Black);

        board = board.withMove (Color::White, e2e4);
        board = board.withMove (Color::Black, e7e5);
        board = board.withMove (Color::White, f1c4);
        board = board.withMove (Color::Black, d7d5);
        board = board.withMove (Color::White, g1f3);
        board = board.withMove (Color::Black, g8f6);

        Coord e1 = toCoord ("e1");
        Coord g1 = toCoord ("g1");
        optional<Move> white_result = mapCoordinatesToMove (board, Color::White, e1, g1);

        Move white_expected = toMove ("o-o", Color::White);
        CHECK( white_result.has_value() );
        CHECK( *white_result == white_expected );

        board = board.withMove (Color::White, white_expected);

        Coord e8 = toCoord ("e8");
        Coord g8 = toCoord ("g8");

        optional<Move> black_result = mapCoordinatesToMove (board, Color::Black, e8, g8);
        Move black_expected = toMove ("o-o", Color::Black);

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

        Coord b7 = toCoord ("b7");
        Coord b8 = toCoord ("b8");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, b7, b8, Piece::Queen);
        auto expected = toMove ("b7b8 (Q)", Color::White);

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

        Coord b7 = toCoord ("b7");
        Coord c8 = toCoord ("c8");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, b7, c8, Piece::Queen);
        auto expected = toMove ("b7xc8 (Q)", Color::White);

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

        Coord f1 = toCoord ("f1");
        Coord c4 = toCoord ("c4");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, f1, c4);
        auto expected = toMove ("f1xc4", Color::White);

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

        Coord f1 = toCoord ("f1");
        Coord c3 = toCoord ("c3");

        optional<Move> result = mapCoordinatesToMove (board, Color::White, f1, c3);
        auto expected = toMove ("f1 c3", Color::White);

        CHECK( result.has_value() );
        CHECK( *result == expected );
    }
}
