#include <catch/catch.hpp>

#include "board.h"

TEST_CASE( "Pawn promotion works on appropriate rows", "[pawn promotion]" )
{
    CHECK( need_pawn_promotion (0, Color::Black) == false );
    CHECK( need_pawn_promotion (0, Color::White) == true );
    CHECK( need_pawn_promotion (7, Color::Black) == true );
    CHECK( need_pawn_promotion (7, Color::White) == false );
}

TEST_CASE( "Pawn promotion doesn't happen on other rows but the last", "[pawn promotion]" )
{
    auto who = GENERATE( Color::White, Color::Black );
    auto row = GENERATE( range(1, 6) );

    CHECK( need_pawn_promotion (row, who) == false );
}

