#include "catch.hpp"

#include <algorithm>
#include <iostream>

#include "board_code.hpp"

TEST_CASE( "Board code is able to be set", "[board-code]" )
{
    board_code code, initial;

    auto initial_str = code.to_string ();
    std::size_t num_zeroes = std::count (initial_str.begin(), initial_str.end(), '0');
    REQUIRE( num_zeroes == initial_str.size () );

    coord_t a8 = coord_parse ("a8");
    piece_t black_pawn = make_piece (Color::Black, Piece::Pawn);
    code.add_piece (a8, black_pawn);

    REQUIRE( code != initial );

    code.remove_piece (a8);

    REQUIRE ( code == initial );

    coord_t h1 = coord_parse ("h1");
    piece_t white_king = make_piece (Color::White, Piece::King);
    code.add_piece (h1, white_king);

    std::string result = code.to_string();
    result = result.substr (0, 4);
    REQUIRE( result == "0001" );
    code.remove_piece (h1);
    result = code.to_string().substr(0, 4);
    REQUIRE( result == "0000" );
}
