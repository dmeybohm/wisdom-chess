#include <random>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "board_builder" )
{
    SUBCASE( "Specifying coordinates in algebraic notation" )
    {
        CHECK( coordRow (coordParse ("a8")) == 0 );
        CHECK( coordRow (coordParse ("a1")) == 7 );
        CHECK( coordColumn (coordParse ("a8")) == 0 );
        CHECK( coordColumn (coordParse ("a1")) == 0 );
        CHECK( coordRow (coordParse ("h1")) == 7 );
        CHECK( coordRow (coordParse ("h8")) == 0 );
        CHECK( coordColumn (coordParse ("h1")) == 7 );
        CHECK( coordColumn (coordParse ("h8")) == 7 );
    }

    SUBCASE( "Initializing the board builder" )
    {
        BoardBuilder builder;

        builder.addPiece ("a7", Color::White, Piece::Pawn);
        builder.addPiece ("g8", Color::White, Piece::King);
        builder.addPiece ("a1", Color::Black, Piece::King);

        auto board = Board { builder };

        ColoredPiece pawn = board.pieceAt (1, 0);
        ColoredPiece white_king = board.pieceAt (0, 6);
        ColoredPiece black_king = board.pieceAt (7, 0);
        ColoredPiece center = board.pieceAt (4, 5);

        CHECK( pieceColor (pawn) == Color::White );
        CHECK( pieceColor (white_king) == Color::White );
        CHECK( pieceColor (black_king) == Color::Black );
        CHECK( pieceColor (center) == Color::None );

        CHECK( pieceType (pawn) == Piece::Pawn );
        CHECK( pieceType (white_king) == Piece::King );
        CHECK( pieceType (black_king) == Piece::King );
        CHECK( pieceType (center) == Piece::None );
    }

    SUBCASE( "Board builder throws exception for invalid coordinate" )
    {
        BoardBuilder builder;

        REQUIRE_THROWS_WITH_AS(
            builder.addPiece ("a9", Color::White, Piece::Pawn),
            "Invalid coordinate!",
            CoordParseError
        );

        REQUIRE_THROWS_WITH_AS(
            builder.addPiece ("j7", Color::Black, Piece::Bishop),
            "Invalid coordinate!" ,
            CoordParseError
        );

        REQUIRE_THROWS_WITH_AS(
            builder.addPiece ("j9", Color::White, Piece::King),
            "Invalid coordinate!",
            CoordParseError
        );

        REQUIRE_THROWS_WITH_AS(
            builder.addPiece ("asdf", Color::White, Piece::King),
            "Invalid coordinate string!",
            BoardBuilderError
        );
    }
}

TEST_CASE( "Board builder rejects move clocks that are out of range" )
{
    BoardBuilder builder;

    CHECK_THROWS_AS( builder.setHalfMovesClock (-1), BoardBuilderError );
    CHECK_THROWS_AS( builder.setFullMoves (-1), BoardBuilderError );
    CHECK_THROWS_AS( builder.setHalfMovesClock (Max_Half_Move_Clock + 1), BoardBuilderError );
    CHECK_THROWS_AS( builder.setFullMoves (Max_Full_Move_Number + 1), BoardBuilderError );

    builder.setHalfMovesClock (0);
    builder.setFullMoves (0);
    CHECK( builder.getHalfMoveClock() == 0 );
    CHECK( builder.getFullMoveClock() == 0 );

    builder.setHalfMovesClock (Max_Half_Move_Clock);
    builder.setFullMoves (Max_Full_Move_Number);
    CHECK( builder.getHalfMoveClock() == Max_Half_Move_Clock );
    CHECK( builder.getFullMoveClock() == Max_Full_Move_Number );
}

