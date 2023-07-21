#include <doctest/doctest.h>

#include "coord.hpp"

using namespace wisdom;

TEST_CASE( "A coordinate can be generated" )
{
    for (int8_t row = 0; row < 8; row++) {
        for (int8_t col = 0; col < 8; col++) {
            Coord coord = makeCoord (row, col);

            CHECK( Row (coord) == row );
            CHECK( Column (coord) == col );
        }
    }
}


TEST_CASE( "Coord_parse specifying coordinates in algebraic notation" )
{
    CHECK( Row (coordParse ("a8")) == 0 );
    CHECK( Row (coordParse ("a1")) == 7 );
    CHECK( Column (coordParse ("a8")) == 0 );
    CHECK( Column (coordParse ("a1")) == 0 );
    CHECK( Row (coordParse ("h1")) == 7 );
    CHECK( Row (coordParse ("h8")) == 0 );
    CHECK( Column (coordParse ("h1")) == 7 );
    CHECK( Column (coordParse ("h8")) == 7 );
}