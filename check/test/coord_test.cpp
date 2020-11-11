#include "catch.hpp"

#include "coord.h"

TEST_CASE( "A coordinate can be generated", "[coord]" )
{
    auto row = GENERATE( range(0, 7) );
    auto col = GENERATE( range(0, 7) );

    coord_t coord = coord_create( row, col );

    CHECK( ROW(coord) == row );
    CHECK( COLUMN(coord) == col );
}