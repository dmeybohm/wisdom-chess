#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/coord_set.hpp"

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
    CHECK( coordRow (toCoord ("a8")) == 0 );
    CHECK( coordRow (toCoord ("a1")) == 7 );
    CHECK( coordColumn (toCoord ("a8")) == 0 );
    CHECK( coordColumn (toCoord ("a1")) == 0 );
    CHECK( coordRow (toCoord ("h1")) == 7 );
    CHECK( coordRow (toCoord ("h8")) == 0 );
    CHECK( coordColumn (toCoord ("h1")) == 7 );
    CHECK( coordColumn (toCoord ("h8")) == 7 );
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
        CoordIterator iterator { toCoord ("a1") };

        CHECK( *iterator.begin() == toCoord ("a1") );
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

TEST_CASE( "CoordSet" )
{
    SUBCASE( "An empty set visits nothing" )
    {
        CoordSet empty;

        CHECK( empty.size() == 0 );
        CHECK( empty.begin() == empty.end() );
    }

    SUBCASE( "A set visits its squares in index order" )
    {
        auto bits = CoordSet::bit (toCoord ("h1"))
            | CoordSet::bit (toCoord ("a8"))
            | CoordSet::bit (toCoord ("e4"));
        CoordSet set { bits };

        vector<Coord> visited;
        for (auto coord : set)
            visited.push_back (coord);

        CHECK( set.size() == 3 );
        CHECK( visited == vector { toCoord ("a8"), toCoord ("e4"), toCoord ("h1") } );
    }

    SUBCASE( "contains() tests one square" )
    {
        CoordSet set { CoordSet::bit (toCoord ("d5")) };

        CHECK( set.contains (toCoord ("d5")) );
        CHECK( !set.contains (toCoord ("d4")) );
    }
}

TEST_CASE( "Parsing a coordinate without exceptions" )
{
    CHECK( parseCoord ("e2") == toCoord ("e2") );
    CHECK( parseCoord ("h8") == toCoord ("h8") );

    CHECK( !parseCoord ("").has_value() );
    CHECK( !parseCoord ("e").has_value() );
    CHECK( !parseCoord ("e22").has_value() );
    CHECK( !parseCoord ("z9").has_value() );
    CHECK( !parseCoord ("i1").has_value() );
    CHECK( !parseCoord ("a0").has_value() );
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

    auto d5 = toCoord ("d5");
    auto e5 = toCoord ("e5");
    auto d4 = toCoord ("d4");
    auto e4 = toCoord ("e4");

    CHECK( coordColor (d5) == Color::White );
    CHECK( coordColor (e4) == Color::White );
    CHECK( coordColor (e5) == Color::Black );
    CHECK( coordColor (d4) == Color::Black );
}

TEST_CASE( "isDarkSquare()" )
{
    CHECK( !isDarkSquare (toCoord ("a8")) );
    CHECK( isDarkSquare (toCoord ("b8")) );
    CHECK( isDarkSquare (toCoord ("a1")) );
    CHECK( !isDarkSquare (toCoord ("h1")) );
    CHECK( isDarkSquare (toCoord ("d4")) );
    CHECK( !isDarkSquare (toCoord ("e4")) );

    SUBCASE( "Neighboring squares alternate" )
    {
        for (auto coord : CoordIterator {})
        {
            if (coord.column() + 1 < Num_Columns)
            {
                auto right = makeCoord (coord.row(), coord.column() + 1);
                CHECK( isDarkSquare (coord) != isDarkSquare (right) );
            }
        }
    }
}

TEST_CASE( "Pawn direction is negative for white and positive for black" )
{
    CHECK( pawnDirection (Color::White) == -1 );
    CHECK( pawnDirection (Color::Black) == +1 );
}
