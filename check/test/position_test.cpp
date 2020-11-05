#include "catch.hpp"
#include "board_builder.hpp"

extern "C"
{
#include "board.h"
}

TEST_CASE("Position is initialized correctly", "[position-test]")
{
    struct board *board = board_new();

    CHECK( board->position.score[COLOR_INDEX_WHITE] < 0 );
    CHECK( board->position.score[COLOR_INDEX_BLACK] < 0 );
    CHECK( board->position.score[COLOR_INDEX_WHITE] ==
           board->position.score[COLOR_INDEX_BLACK]);
}

TEST_CASE("Center pawn elevates position score", "[position-test]")
{
    board_builder builder;

    builder.add_piece("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece("e8", COLOR_BLACK, PIECE_KING);

    builder.add_piece("e4", COLOR_WHITE, PIECE_PAWN);
    builder.add_piece("a6", COLOR_BLACK, PIECE_PAWN);
    struct board *board = builder.build();

    CHECK( board->position.score[COLOR_INDEX_WHITE] > board->position.score[COLOR_INDEX_BLACK]);
}