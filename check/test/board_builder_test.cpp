#include "tests.hpp"
#include "board_builder.hpp"
#include "board.hpp"

using namespace wisdom;

TEST_CASE( "Specifying coordinates in algebraic notation" )
{
    CHECK( ROW(coord_alg("a8")) == 0 );
    CHECK( ROW(coord_alg("a1")) == 7 );
    CHECK( COLUMN(coord_alg("a8")) == 0 );
    CHECK( COLUMN(coord_alg("a1")) == 0 );
    CHECK( ROW(coord_alg("h1")) == 7 );
    CHECK( ROW(coord_alg("h8")) == 0 );
    CHECK( COLUMN(coord_alg("h1")) == 7 );
    CHECK( COLUMN(coord_alg("h8")) == 7 );
}

TEST_CASE( "Initializing the board builder" )
{
    BoardBuilder builder;

    builder.add_piece ("a7", Color::White, Piece::Pawn);
    builder.add_piece ("g8", Color::White, Piece::King);
    builder.add_piece ("a1", Color::Black, Piece::King);

    Board board = builder.build();

    ColoredPiece pawn = piece_at (board, 1, 0);
    ColoredPiece white_king = piece_at (board, 0, 6);
    ColoredPiece black_king = piece_at (board, 7, 0);
    ColoredPiece center = piece_at (board, 4, 5);

    CHECK( piece_color (pawn) == Color::White );
    CHECK( piece_color (white_king) == Color::White );
    CHECK( piece_color (black_king) == Color::Black );
    CHECK( piece_color (center) == Color::None );

    CHECK( piece_type (pawn) == Piece::Pawn );
    CHECK( piece_type (white_king) == Piece::King );
    CHECK( piece_type (black_king) == Piece::King );
    CHECK( piece_type (center) == Piece::None );
}

TEST_CASE( "Board builder throws exception for invalid coordinate" )
{
    BoardBuilder builder;
    bool no_throw = false;

    try {
        builder.add_piece ("a9", Color::White, Piece::Pawn);
        no_throw = true;
    } catch (const BoardBuilderError &board_builder_exception) {
        CHECK( board_builder_exception.message() == "Invalid row!" );
    }
    REQUIRE( no_throw == false );

    try {
        builder.add_piece ("j7", Color::White, Piece::Pawn);
        no_throw = true;
    } catch (const BoardBuilderError &board_builder_exception) {
        CHECK( board_builder_exception.message() == "Invalid column!" );
    }
    REQUIRE( no_throw == false );

    try {
        builder.add_piece ("asdf", Color::White, Piece::Pawn);
        no_throw = true;
    } catch (const BoardBuilderError &board_builder_exception) {
        CHECK( board_builder_exception.message() == "Invalid coordinate string!" );
    }
    REQUIRE( no_throw == false );
}

