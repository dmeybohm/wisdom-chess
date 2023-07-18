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
        CHECK(Row (coord_parse("a8")) == 0 );
        CHECK(Row (coord_parse("a1")) == 7 );
        CHECK(Column (coord_parse("a8")) == 0 );
        CHECK(Column (coord_parse("a1")) == 0 );
        CHECK(Row (coord_parse("h1")) == 7 );
        CHECK(Row (coord_parse("h8")) == 0 );
        CHECK(Column (coord_parse("h1")) == 7 );
        CHECK(Column (coord_parse("h8")) == 7 );
    }

    SUBCASE( "Initializing the board builder" )
    {
        BoardBuilder builder;

        builder.add_piece ("a7", Color::White, Piece::Pawn);
        builder.add_piece ("g8", Color::White, Piece::King);
        builder.add_piece ("a1", Color::Black, Piece::King);

        auto board = Board { builder };

        ColoredPiece pawn = board.pieceAt (1, 0);
        ColoredPiece white_king = board.pieceAt (0, 6);
        ColoredPiece black_king = board.pieceAt (7, 0);
        ColoredPiece center = board.pieceAt (4, 5);

        CHECK( piece_color (pawn) == Color::White );
        CHECK( piece_color (white_king) == Color::White );
        CHECK( piece_color (black_king) == Color::Black );
        CHECK( piece_color (center) == Color::None );

        CHECK( piece_type (pawn) == Piece::Pawn );
        CHECK( piece_type (white_king) == Piece::King );
        CHECK( piece_type (black_king) == Piece::King );
        CHECK( piece_type (center) == Piece::None );
    }

    SUBCASE( "Board builder throws exception for invalid coordinate" )
    {
        BoardBuilder builder;
        bool no_throw = false;

        try {
            builder.add_piece ("a9", Color::White, Piece::Pawn);
            no_throw = true;
        } catch (const CoordParseError& board_builder_exception) {
            CHECK( board_builder_exception.message() == "Invalid coordinate!" );
        }
        REQUIRE( no_throw == false );

        try {
            builder.add_piece ("j7", Color::White, Piece::Pawn);
            no_throw = true;
        } catch (const CoordParseError& board_builder_exception) {
            CHECK( board_builder_exception.message() == "Invalid coordinate!" );
        }
        REQUIRE( no_throw == false );

        try {
            builder.add_piece ("asdf", Color::White, Piece::Pawn);
            no_throw = true;
        } catch (const BoardBuilderError& board_builder_exception) {
            CHECK( board_builder_exception.message() == "Invalid coordinate string!" );
        }
        REQUIRE( no_throw == false );
    }
}

TEST_CASE( "Board can be randomized" )
{
    Board randomized_board;
    Board default_board;

    randomized_board.randomizePositions ();

    SUBCASE("Board code is not the same as the default" )
    {
        REQUIRE(default_board.getCode () != randomized_board.getCode () );
    }

    SUBCASE("None of the pawns are in the back row" )
    {
        for (int8_t col = 0; col < Num_Columns; col++)
        {
            auto first_row_piece = randomized_board.pieceAt (7, col);
            auto last_row_piece = randomized_board.pieceAt (0, col);

            CHECK( piece_type (last_row_piece) != Piece::Pawn );
            CHECK( piece_type (first_row_piece) != Piece::Pawn );
        }
    }

    SUBCASE("Both kings are not in check" )
    {
        auto white_king_pos = randomized_board.getKingPosition (Color::White);
        auto black_king_pos = randomized_board.getKingPosition (Color::Black);
        auto white_in_check = is_king_threatened (randomized_board, Color::White, white_king_pos);
        auto black_in_check = is_king_threatened (randomized_board, Color::Black, black_king_pos);

        auto invariant = !white_in_check || !black_in_check;
        REQUIRE( invariant );
    }
}
