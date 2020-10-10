#include "catch.hpp"
#include "board_builder.hpp"

extern "C"
{
#include "../src/board.h"
}

TEST_CASE( "Initializing the board builder", "[board-builder]" )
{
    board_builder builder;

    builder.add_piece ("a7", COLOR_WHITE, PIECE_PAWN);
    builder.add_piece ("g8", COLOR_WHITE, PIECE_KING);
    builder.add_piece ("a1", COLOR_BLACK, PIECE_KING);

    struct board *board = builder.build();

    piece_t pawn = PIECE_AT (board, 1, 0);
    piece_t white_king = PIECE_AT (board, 0, 6);
    piece_t black_king = PIECE_AT (board, 7, 0);
    piece_t center = PIECE_AT (board, 4, 5);

    CHECK( PIECE_COLOR(pawn) == COLOR_WHITE );
    CHECK( PIECE_COLOR(white_king) == COLOR_WHITE );
    CHECK( PIECE_COLOR(black_king) == COLOR_BLACK );
    CHECK( PIECE_COLOR(center) == COLOR_NONE );

    CHECK( PIECE_TYPE(pawn) == PIECE_PAWN );
    CHECK( PIECE_TYPE(white_king) == PIECE_KING );
    CHECK( PIECE_TYPE(black_king) == PIECE_KING );
    CHECK( PIECE_TYPE(center) == PIECE_NONE );
}

TEST_CASE( "Board builder throws exception for invalid coordinate", "[board-builder]" )
{
    board_builder builder;
    bool no_throw = false;
    try {
        builder.add_piece ("a9", COLOR_WHITE, PIECE_PAWN);
        no_throw = true;
    } catch (const board_builder_exception &board_builder_exception) {
        CHECK( board_builder_exception.what() != nullptr );
    }
    REQUIRE( no_throw == false );
}

