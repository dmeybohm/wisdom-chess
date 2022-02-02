#include <doctest/doctest.h>

#include "coord.hpp"

using namespace wisdom;

TEST_CASE( "A coordinate can be generated" )
{
    for (int8_t row = 0; row < 8; row++) {
        for (int8_t col = 0; col < 8; col++) {
            Coord coord = make_coord (row, col);

            CHECK(Row (coord) == row );
            CHECK(Column (coord) == col );
        }
    }
}


TEST_CASE( "Coord_parse specifying coordinates in algebraic notation" )
{
    CHECK(Row (coord_parse ("a8")) == 0 );
    CHECK(Row (coord_parse ("a1")) == 7 );
    CHECK(Column (coord_parse ("a8")) == 0 );
    CHECK(Column (coord_parse ("a1")) == 0 );
    CHECK(Row (coord_parse ("h1")) == 7 );
    CHECK(Row (coord_parse ("h8")) == 0 );
    CHECK(Column (coord_parse ("h1")) == 7 );
    CHECK(Column (coord_parse ("h8")) == 7 );
}