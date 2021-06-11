#include <doctest/doctest.h>

#include "coord.hpp"

using namespace wisdom;

TEST_CASE( "A coordinate can be generated" )
{
    for (auto row = 0; row < 8; row++) {
        for (auto col = 0; col < 8; col++) {
            Coord coord = make_coord (row, col);

            CHECK( ROW(coord) == row );
            CHECK( COLUMN(coord) == col );
        }
    }
}


TEST_CASE( "Coord_parse specifying coordinates in algebraic notation" )
{
    CHECK( ROW(coord_parse("a8")) == 0 );
    CHECK( ROW(coord_parse("a1")) == 7 );
    CHECK( COLUMN(coord_parse("a8")) == 0 );
    CHECK( COLUMN(coord_parse("a1")) == 0 );
    CHECK( ROW(coord_parse("h1")) == 7 );
    CHECK( ROW(coord_parse("h8")) == 0 );
    CHECK( COLUMN(coord_parse("h1")) == 7 );
    CHECK( COLUMN(coord_parse("h8")) == 7 );
}