#include <catch/catch.hpp>

#include "coord.h"

TEST_CASE( "A coordinate can be generated", "[coord]" )
{
    auto row = GENERATE( range(0, 7) );
    auto col = GENERATE( range(0, 7) );

    coord_t coord = make_coord (row, col);

    CHECK( ROW(coord) == row );
    CHECK( COLUMN(coord) == col );
}


TEST_CASE( "Coord_parse specifying coordinates in algebraic notation", "[coord]" )
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