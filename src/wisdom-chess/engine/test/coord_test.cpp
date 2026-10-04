#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "A coordinate can be generated" )
{
    for (int8_t row = 0; row < 8; row++)
    {
        for (int8_t col = 0; col < 8; col++)
        {
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

TEST_CASE( "Parsing a coordinate without exceptions" )
{
    CHECK( coordParseOptional ("e2") == coordParse ("e2") );
    CHECK( coordParseOptional ("h8") == coordParse ("h8") );

    CHECK( !coordParseOptional ("").has_value() );
    CHECK( !coordParseOptional ("e").has_value() );
    CHECK( !coordParseOptional ("e22").has_value() );
    CHECK( !coordParseOptional ("z9").has_value() );
    CHECK( !coordParseOptional ("i1").has_value() );
    CHECK( !coordParseOptional ("a0").has_value() );
}

TEST_CASE( "coordColor()" )
{
    auto top_left = makeCoord (First_Row, First_Column);
    auto bottom_right = makeCoord (Last_Row, Last_Column);
    auto top_right = makeCoord (First_Row, Last_Column);
    auto bottom_left = makeCoord (Last_Row, First_Column);

    CHECK( coordColor (top_left) == Color::White );
    CHECK( coordColor (bottom_right) == Color::White );
    CHECK( coordColor (top_right) == Color::Black );
    CHECK( coordColor (bottom_left) == Color::Black );

    auto d5 = coordParse ("d5");
    auto e5 = coordParse ("e5");
    auto d4 = coordParse ("d4");
    auto e4 = coordParse ("e4");

    CHECK( coordColor (d5) == Color::White );
    CHECK( coordColor (e4) == Color::White );
    CHECK( coordColor (e5) == Color::Black );
    CHECK( coordColor (d4) == Color::Black );
}

TEST_CASE( "Pawn direction is negative for white and positive for black" )
{
    CHECK( pawnDirection (Color::White) == -1 );
    CHECK( pawnDirection (Color::Black) == +1 );
}
