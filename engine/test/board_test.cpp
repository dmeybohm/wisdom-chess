#include "tests.hpp"
#include "board.hpp"
#include "piece.hpp"

using namespace wisdom;

TEST_CASE("Pawn direction is negative for white and positive for black")
{
    CHECK( pawn_direction (Color::White) == -1 );
    CHECK( pawn_direction (Color::Black) == +1 );
}

