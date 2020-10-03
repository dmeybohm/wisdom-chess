#include "catch.hpp"

extern "C"
{
#include "../src/piece.h"
}

TEST_CASE( "A piece can be converted", "[piece]" )
{
    auto color = GENERATE (COLOR_WHITE, COLOR_BLACK);
    auto piece = GENERATE (PIECE_NONE, PIECE_PAWN, PIECE_BISHOP, PIECE_KNIGHT, PIECE_ROOK, PIECE_QUEEN, PIECE_KING);

    piece_t combined = MAKE_PIECE (color, piece);
    CHECK( PIECE_TYPE(combined) == piece );
    CHECK( PIECE_COLOR(combined) == color );
}