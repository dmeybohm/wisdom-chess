#include <doctest/doctest.h>
#include "move.hpp"

using namespace wisdom;

TEST_CASE( "move_parse" )
{
    SUBCASE( "move_parse parses captures and non-captures" )
    {
        Move capture = move_parse ("a6xb7");
        Move non_capture = move_parse ("e4 e8");

        CHECK( move_equals (capture, move_parse ("a6xb7", Color::White)) );
        CHECK( move_equals (non_capture, move_parse ("e4 e8", Color::White)) );
    }

    SUBCASE( "color matters in move_parse" )
    {
        Move castle = move_parse ("o-o", Color::Black);
        CHECK( Row (move_src (castle)) == 0 );
        CHECK( Row (move_dst (castle)) == 0 );
        CHECK( move_equals (castle, move_parse ("o-o", Color::Black)) );
    }

    SUBCASE( "move_parse throws an exception for castling moves" )
    {
        CHECK_THROWS_AS( (void)move_parse ("o-o"), ParseMoveException );
        CHECK_THROWS_WITH( (void)move_parse ("o-o"), "Move requires color, but no color provided" );
        CHECK_THROWS_WITH( (void)move_parse ("o-o-o"), "Move requires color, but no color provided" );
    }

    SUBCASE( "Invalid moves throw an exception" )
    {
        CHECK_THROWS_AS( (void)move_parse ("invalid"), ParseMoveException );
        CHECK_THROWS_WITH( (void)move_parse ("invalid"), "Error parsing move: invalid" );
    }
}