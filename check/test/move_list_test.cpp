#include "catch.hpp"

#include "move_list.hpp"

TEST_CASE( "Initializing move list", "[move-list]")
{
    move_list_t move_list { COLOR_BLACK, {"e4 d4", "d2 d1"}};
    REQUIRE( move_list.size() == 2 );

    std::vector<move_t> moves;
    for (auto move : move_list)
    {
        moves.push_back(move);
    }
    std::vector expected = { parse_move("e4 d4"), parse_move("d2 d1")};
    REQUIRE( moves == expected );
}
