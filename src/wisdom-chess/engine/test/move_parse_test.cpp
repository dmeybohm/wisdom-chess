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

    SUBCASE( "Castling needs a color" )
    {
        CHECK( !moveParseOptional ("o-o", Color::None).has_value() );
        CHECK( !moveParseOptional ("o-o-o", Color::None).has_value() );
    }

    SUBCASE( "Invalid moves are rejected" )
    {
        CHECK( !moveParseOptional ("invalid", Color::White).has_value() );
    }

    SUBCASE( "Empty and whitespace-only input" )
    {
        CHECK( !moveParseOptional ("", Color::White).has_value() );
        CHECK( !moveParseOptional ("  \t ", Color::White).has_value() );
    }

    SUBCASE( "Bytes above 0x7f are rejected rather than classified" )
    {
        CHECK( !moveParseOptional ("\xe9", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xc3\xa9", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xa0" "e2e4", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xd0-\xd0", Color::White).has_value() );
        CHECK( !moveParseOptional ("\xff", Color::White).has_value() );
    }
}
