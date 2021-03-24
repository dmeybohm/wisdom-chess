#include "doctest/doctest.h"

#include "board.h"

TEST_CASE( "Pawn promotion works on appropriate rows" )
{
    CHECK( need_pawn_promotion (0, Color::Black) == false );
    CHECK( need_pawn_promotion (0, Color::White) == true );
    CHECK( need_pawn_promotion (7, Color::Black) == true );
    CHECK( need_pawn_promotion (7, Color::White) == false );
}

TEST_CASE( "Pawn promotion doesn't happen on other rows but the last" )
{
    std::vector colors { Color::White, Color::Black };
    for (auto who : colors)
    {
        for (auto row = 1; row <= 6; row++)
        {
            CHECK( need_pawn_promotion (row, who) == false );
        }
    }
}

