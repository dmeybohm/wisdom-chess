#include "catch.hpp"

#include <algorithm>
#include <iostream>

#include "board_code.hpp"

TEST_CASE( "Board code is able to be set", "[board-code]" )
{
    board_code code, initial;

    auto initial_str = code.to_string ();
    auto num_zeroes = std::count (initial_str.begin(), initial_str.end(), '0');
    REQUIRE( num_zeroes == initial_str.size () );

    coord_t a8 = coord_parse ("a8");
    piece_t black_king = make_piece (Color::Black, Piece::Pawn);
    code.add_piece (a8, black_king);

    REQUIRE( code != initial );

    code.remove_piece (a8);

    REQUIRE ( code == initial );
}
