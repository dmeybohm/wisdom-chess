#include "wisdom-chess/engine/move.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "moveParse" )
{
    SUBCASE( "moveParse parses captures and non-captures" )
    {
        Move capture = moveParse ("a6xb7");
        Move non_capture = moveParse ("e4 e8");

        CHECK( ( capture == moveParse ("a6xb7", Color::White) ) );
        CHECK( ( non_capture == moveParse ("e4 e8", Color::White) ) );
    }

    SUBCASE( "color matters in moveParse" )
    {
        Move castle = moveParse ("o-o", Color::Black);
        CHECK( ( coordRow (castle.getSrc()) == 0 ) );
        CHECK( ( coordRow (castle.getDst()) == 0 ) );
        CHECK( ( castle == moveParse ("o-o", Color::Black) ) );
    }

    SUBCASE( "moveParse throws an exception for castling moves" )
    {
        CHECK_THROWS_AS( (void)moveParse ("o-o"), ParseMoveException );
        CHECK_THROWS_WITH( (void)moveParse ("o-o"), "Move requires color, but no color provided" );
        CHECK_THROWS_WITH( (void)moveParse ("o-o-o"), "Move requires color, but no color provided" );
    }

    SUBCASE( "Invalid moves throw an exception" )
    {
        CHECK_THROWS_AS( (void)moveParse ("invalid"), ParseMoveException );
        CHECK_THROWS_WITH( (void)moveParse ("invalid"), "Error parsing move: invalid" );
    }

    SUBCASE( "Empty and whitespace-only input" )
    {
        CHECK_THROWS_AS( (void)moveParse (""), ParseMoveException );
        CHECK_THROWS_WITH( (void)moveParse (""), "Error parsing move: empty string" );
        CHECK_THROWS_AS( (void)moveParse ("", Color::White), ParseMoveException );
        CHECK_THROWS_AS( (void)moveParse ("   "), ParseMoveException );

        CHECK( !moveParseOptional ("", Color::White).has_value() );
        CHECK( !moveParseOptional ("  \t ", Color::White).has_value() );
    }

    SUBCASE( "Bytes above 0x7f are rejected rather than classified" )
    {
        CHECK( !moveParseOptional ("\xe9", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xc3\xa9", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xa0" "e2e4", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xd0-\xd0", Color::White).has_value() );
        CHECK_THROWS_AS( (void)moveParse ("\xff", Color::White), ParseMoveException );
    }
}
