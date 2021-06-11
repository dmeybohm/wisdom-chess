#include <doctest/doctest.h>
#include "move.hpp"

using namespace wisdom;

TEST_CASE("parse_simple_move parses captures and non-captures")
{
    Move capture = parse_move("a6xb7");
    Move non_capture = parse_move("e4 e8");

    REQUIRE( move_equals (capture, move_parse ("a6xb7", Color::White)) );
    REQUIRE( move_equals (non_capture, move_parse ("e4 e8", Color::White)) );
}

TEST_CASE("color matters in parse_move")
{
    Move castle = parse_move("o-o", Color::Black);
    REQUIRE( ROW(move_src(castle)) == 0 );
    REQUIRE( ROW(move_dst(castle)) == 0 );
    REQUIRE( move_equals (castle, move_parse ("o-o", Color::Black)));
}

TEST_CASE("parse_move throws an exception for en-passant moves")
{
    REQUIRE_THROWS_AS( parse_move ("d5 d5 ep"), ParseMoveException );
    REQUIRE_THROWS_WITH( parse_move ("d4 d5 ep"), "Invalid type of move in parse_simple_move");
}

TEST_CASE("parse_move throws an exception for castling moves")
{
    REQUIRE_THROWS_AS( parse_move ("o-o"), ParseMoveException );
    REQUIRE_THROWS_WITH( parse_move ("o-o"), "Move requires color, but no color provided");
    REQUIRE_THROWS_WITH( parse_move ("o-o-o"), "Move requires color, but no color provided");
}

TEST_CASE("Invalid moves throw an exception")
{
    REQUIRE_THROWS_AS( parse_move ("invalid"), ParseMoveException );
    REQUIRE_THROWS_WITH( parse_move ("invalid"), "Error parsing move: invalid" );
}
