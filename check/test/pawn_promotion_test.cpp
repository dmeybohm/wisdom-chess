#include "catch.hpp"

extern "C"
{
#include "board.h"
}

TEST_CASE( "Pawn promotion works on appropriate rows", "[pawn promotion]" )
{
    CHECK( need_pawn_promotion (0, COLOR_BLACK) == false );
    CHECK( need_pawn_promotion (0, COLOR_WHITE) == true );
    CHECK( need_pawn_promotion (7, COLOR_BLACK) == true );
    CHECK( need_pawn_promotion (7, COLOR_WHITE) == false );
}

TEST_CASE( "Pawn promotion doesn't happen on other rows but the last", "[pawn promotion]" )
{
    color_t who = GENERATE( COLOR_WHITE, COLOR_BLACK );
    uint8_t row = GENERATE( range(1, 6) );

    CHECK( need_pawn_promotion (row, who) == false );
}

