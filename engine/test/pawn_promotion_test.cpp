#include <doctest/doctest.h>

#include "board.hpp"

using namespace wisdom;

TEST_CASE( "Pawn promotion works on appropriate rows" )
{
    CHECK( needPawnPromotion (0, Color::Black) == false );
    CHECK( needPawnPromotion (0, Color::White) == true );
    CHECK( needPawnPromotion (7, Color::Black) == true );
    CHECK( needPawnPromotion (7, Color::White) == false );
}

TEST_CASE( "Pawn promotion doesn't happen on other rows but the last" )
{
    std::vector colors { Color::White, Color::Black };
    for (auto who : colors)
    {
        for (auto row = 1; row <= 6; row++)
        {
            CHECK( needPawnPromotion (row, who) == false );
        }
    }
}

