#include "wisdom-chess/engine/move.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "toMove" )
{
    SUBCASE( "toMove parses captures and non-captures" )
    {
        Move capture = toMove ("a6xb7");
        Move non_capture = toMove ("e4 e8");

        CHECK( ( capture == toMove ("a6xb7", Color::White) ) );
        CHECK( ( non_capture == toMove ("e4 e8", Color::White) ) );
    }

    SUBCASE( "color matters in toMove" )
    {
        Move castle = toMove ("o-o", Color::Black);
        CHECK( ( coordRow (castle.getSrc()) == 0 ) );
        CHECK( ( coordRow (castle.getDst()) == 0 ) );
        CHECK( ( castle == toMove ("o-o", Color::Black) ) );
    }

    SUBCASE( "Castling needs a color" )
    {
        CHECK( !parseMove ("o-o", Color::None).has_value() );
        CHECK( !parseMove ("o-o-o", Color::None).has_value() );
    }

    SUBCASE( "Invalid moves are rejected" )
    {
        CHECK( !parseMove ("invalid", Color::White).has_value() );
    }

    SUBCASE( "Empty and whitespace-only input" )
    {
        CHECK( !parseMove ("", Color::White).has_value() );
        CHECK( !parseMove ("  \t ", Color::White).has_value() );
    }

    SUBCASE( "Bytes above 0x7f are rejected rather than classified" )
    {
        CHECK( !parseMove ("\xe9", Color::White).has_value() );
        CHECK( !parseMove ("\xc3\xa9", Color::White).has_value() );
        CHECK( !parseMove ("\xa0" "e2e4", Color::White).has_value() );
        CHECK( !parseMove ("\xd0-\xd0", Color::White).has_value() );
        CHECK( !parseMove ("\xff", Color::White).has_value() );
    }
}
