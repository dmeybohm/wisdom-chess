#include "tests.hpp"
#include <doctest/doctest.h>
#include "board.hpp"
#include "variation_glimpse.hpp"

using namespace wisdom;

TEST_CASE("Variation glimpse without overflow" )
{
    Board board;
    VariationGlimpse glimpse;
    MoveList move_list { Color::White, { "a1 a2", "a3 a4", "a5 a6", "a7 a8",
                                         "b1 b2", "b3 b4", "b5 b6", "b7 b8",
                                         "c1 c2", "c3 c4", "c5 c6", "c7 c8" } };

    for (auto move : move_list) {
        glimpse.push_front (move);
    }
    auto str = glimpse.to_string();
    REQUIRE( str == "{ [c7 c8] [c5 c6] [c3 c4] [c1 c2] [b7 b8] [b5 b6] [b3 b4] [b1 b2] [a7 a8] [a5 a6] [a3 a4] [a1 a2] }" );
}

TEST_CASE("Variation glimpse with overflow" )
{
    Board board;
    VariationGlimpse glimpse;
    MoveList move_list { Color::White, { "a1 a2", "a3 a4", "a5 a6", "a7 a8",
                                         "b1 b2", "b3 b4", "b5 b6", "b7 b8",
                                         "c1 c2", "c3 c4", "c5 c6", "c7 c8" } };

    for (auto move : move_list) {
        glimpse.push_front (move);
    }
    glimpse.push_front (move_parse ("a1 a8"));
    auto str = glimpse.to_string();
    REQUIRE( str == "{ [a1 a8] [c7 c8] [c5 c6] [c3 c4] [c1 c2] [b7 b8] [b5 b6] [b3 b4] [b1 b2] [a7 a8] [a5 a6] [a3 a4] }" );
}