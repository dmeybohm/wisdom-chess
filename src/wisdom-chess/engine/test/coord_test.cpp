#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "A coordinate can be generated" )
{
    for (int8_t row = 0; row < 8; row++) {
        for (int8_t col = 0; col < 8; col++) {
            Coord coord = makeCoord (row, col);

            CHECK( coordRow (coord) == row );
            CHECK( coordColumn (coord) == col );
        }
    }
}


TEST_CASE( "Coord_parse specifying coordinates in algebraic notation" )
{
    CHECK( coordRow (coordParse ("a8")) == 0 );
    CHECK( coordRow (coordParse ("a1")) == 7 );
    CHECK( coordColumn (coordParse ("a8")) == 0 );
    CHECK( coordColumn (coordParse ("a1")) == 0 );
    CHECK( coordRow (coordParse ("h1")) == 7 );
    CHECK( coordRow (coordParse ("h8")) == 0 );
    CHECK( coordColumn (coordParse ("h1")) == 7 );
    CHECK( coordColumn (coordParse ("h8")) == 7 );
}
