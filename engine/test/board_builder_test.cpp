#include "tests.hpp"
#include "board_builder.hpp"
#include "board.hpp"
#include "check.hpp"
#include "coord.hpp"

#include <random>

using namespace wisdom;

TEST_CASE( "board_builder" )
{
    SUBCASE( "Specifying coordinates in algebraic notation" )
    {
        CHECK( Row (coordParse ("a8")) == 0 );
        CHECK( Row (coordParse ("a1")) == 7 );
        CHECK( Column (coordParse ("a8")) == 0 );
        CHECK( Column (coordParse ("a1")) == 0 );
        CHECK( Row (coordParse ("h1")) == 7 );
        CHECK( Row (coordParse ("h8")) == 0 );
        CHECK( Column (coordParse ("h1")) == 7 );
        CHECK( Column (coordParse ("h8")) == 7 );
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

TEST_CASE( "Board can be randomized" )
{
    Board default_board;
    auto randomized_board = default_board.withRandomPosition();

    SUBCASE( "Board code is not the same as the default" )
    {
        REQUIRE( default_board.getCode() != randomized_board.getCode() );
    }

    SUBCASE( "None of the pawns are in the back row" )
    {
        for (int8_t col = 0; col < Num_Columns; col++)
        {
            auto first_row_piece = randomized_board.pieceAt (7, col);
            auto last_row_piece = randomized_board.pieceAt (0, col);

            CHECK( pieceType (last_row_piece) != Piece::Pawn );
            CHECK( pieceType (first_row_piece) != Piece::Pawn );
        }
    }

    SUBCASE( "Both kings are not in check" )
    {
        auto white_king_pos = randomized_board.getKingPosition (Color::White);
        auto black_king_pos = randomized_board.getKingPosition (Color::Black);
        auto white_in_check = isKingThreatened (randomized_board, Color::White, white_king_pos);
        auto black_in_check = isKingThreatened (randomized_board, Color::Black, black_king_pos);

        auto invariant = !white_in_check || !black_in_check;
        REQUIRE( invariant );
    }
}
