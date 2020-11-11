#include "catch.hpp"
#include "parse_simple_move.hpp"

TEST_CASE("parse_simple_move parses captures and non-captures", "[parse-move]")
{
    move_t capture = parse_move("a6xb7");
    move_t non_capture = parse_move("e4 e8");

    REQUIRE( move_equals (capture, move_parse ("a6xb7", COLOR_WHITE)) );
    REQUIRE( move_equals (non_capture, move_parse ("e4 e8", COLOR_WHITE)) );
}

TEST_CASE("color matters in parse_move", "[parse-move]")
{
    move_t castle = parse_move("o-o", COLOR_BLACK);
    REQUIRE( ROW(MOVE_SRC(castle)) == 0 );
    REQUIRE( ROW(MOVE_DST(castle)) == 0 );
    REQUIRE( move_equals (castle, move_parse ("o-o", COLOR_BLACK)));
}

TEST_CASE("parse_move throws an exception for en-passant moves", "[parse-move]")
{
    REQUIRE_THROWS_AS( parse_move ("d5 d5 ep"), parse_move_exception );
    REQUIRE_THROWS_WITH( parse_move ("d4 d5 ep"), Catch::Contains("Invalid type of move"));
}

TEST_CASE("parse_move throws an exception for castling moves", "[parse-move]")
{
    REQUIRE_THROWS_AS( parse_move ("o-o"), parse_move_exception );
    REQUIRE_THROWS_WITH( parse_move ("o-o"), Catch::Contains("Invalid type of move"));
    REQUIRE_THROWS_WITH( parse_move ("o-o-o"), Catch::Contains("Invalid type of move") );
}

TEST_CASE("Invalid moves throw an exception", "[parse-move]")
{
    REQUIRE_THROWS_AS( parse_move ("invalid"), parse_move_exception );
    REQUIRE_THROWS_WITH( parse_move ("invalid"), Catch::Contains("Error parsing move"));
}
