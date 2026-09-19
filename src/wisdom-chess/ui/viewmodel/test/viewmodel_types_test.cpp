#include <doctest/doctest.h>

#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

using namespace wisdom;
using wisdom::ui::getFirstHumanPlayerColor;

TEST_CASE( "getFirstHumanPlayerColor" )
{
    SUBCASE( "White comes first when both players are human" )
    {
        CHECK( getFirstHumanPlayerColor ({ Player::Human, Player::Human }) == Color::White );
    }

    SUBCASE( "A single human player is found on either side" )
    {
        CHECK( getFirstHumanPlayerColor ({ Player::Human, Player::ChessEngine }) == Color::White );
        CHECK( getFirstHumanPlayerColor ({ Player::ChessEngine, Player::Human }) == Color::Black );
    }

    SUBCASE( "Two engines have no human player" )
    {
        CHECK( !getFirstHumanPlayerColor ({ Player::ChessEngine, Player::ChessEngine }).has_value() );
    }
}
