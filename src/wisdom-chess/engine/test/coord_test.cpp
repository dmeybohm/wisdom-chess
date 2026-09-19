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

TEST_CASE( "CoordIterator" )
{
    SUBCASE( "A default iterator visits every square in index order" )
    {
        int expected_index = 0;

        for (auto coord : CoordIterator {})
            CHECK( coord.index() == expected_index++ );

        CHECK( expected_index == Num_Squares );
    }

    SUBCASE( "Iteration begins at the stored coordinate" )
    {
        CoordIterator iterator { coordParse ("a1") };

        CHECK( *iterator.begin() == coordParse ("a1") );
        CHECK( std::distance (iterator.begin(), iterator.end()) == Num_Columns );
    }

    SUBCASE( "Postfix increment returns the previous position" )
    {
        CoordIterator iterator;
        auto previous = iterator++;

        CHECK( *previous == First_Coord );
        CHECK( *iterator == makeCoord (0, 1) );
    }
}
