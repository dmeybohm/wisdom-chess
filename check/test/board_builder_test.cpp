#include "catch.hpp"
#include "board_builder.hpp"

#include "board.h"

using Catch::Matchers::Contains;

TEST_CASE( "Specifying coordinates in algebraic notation", "[board-builder]" )
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

TEST_CASE( "Initializing the board builder", "[board-builder]" )
{
    board_builder builder;

    builder.add_piece ("a7", Color::White, Piece::Pawn);
    builder.add_piece ("g8", Color::White, Piece::King);
    builder.add_piece ("a1", Color::Black, Piece::King);

    struct board board = builder.build();

    piece_t pawn = piece_at (board, 1, 0);
    piece_t white_king = piece_at (board, 0, 6);
    piece_t black_king = piece_at (board, 7, 0);
    piece_t center = piece_at (board, 4, 5);

    CHECK(piece_color (pawn) == Color::White );
    CHECK(piece_color (white_king) == Color::White );
    CHECK(piece_color (black_king) == Color::Black );
    CHECK(piece_color (center) == Color::None );

    CHECK(piece_type (pawn) == Piece::Pawn );
    CHECK(piece_type (white_king) == Piece::King );
    CHECK(piece_type (black_king) == Piece::King );
    CHECK(piece_type (center) == Piece::None );
}

TEST_CASE( "Board builder throws exception for invalid coordinate", "[board-builder]" )
{
    board_builder builder;
    bool no_throw = false;
    try {
        builder.add_piece ("a9", Color::White, Piece::Pawn);
        no_throw = true;
    } catch (const board_builder_exception &board_builder_exception) {
        CHECK( board_builder_exception.what() != nullptr );
        CHECK_THAT( board_builder_exception.what(), Contains("Invalid row") );
    }
    REQUIRE( no_throw == false );

    try {
        builder.add_piece ("j7", Color::White, Piece::Pawn);
        no_throw = true;
    } catch (const board_builder_exception &board_builder_exception) {
        CHECK( board_builder_exception.what() != nullptr );
        CHECK_THAT( board_builder_exception.what(), Contains("Invalid column") );
    }
    REQUIRE( no_throw == false );

    try {
        builder.add_piece ("asdf", Color::White, Piece::Pawn);
        no_throw = true;
    } catch (const board_builder_exception &board_builder_exception) {
        CHECK( board_builder_exception.what() != nullptr );
        CHECK_THAT( board_builder_exception.what(), Contains("Invalid coordinate") );
    }
    REQUIRE( no_throw == false );
}

