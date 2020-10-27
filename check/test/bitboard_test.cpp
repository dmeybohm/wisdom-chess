#include "catch.hpp"
#include "board_builder.hpp"

extern "C"
{
#include "bitboard.h"
}

TEST_CASE( "Bitboard size", "[bitboard]" )
{
    per_player_bitboard_t  bitboard[bits_per_unit(4)];

    CHECK( sizeof(per_player_bitboard_t) == 8 );
    CHECK( sizeof(bitboard) == 8 * 8 * 4 * 2 / 8 );
}

TEST_CASE( "Bitboard get/set", "[bitboard]" )
{
    per_player_bitboard_t  bitboard[bits_per_unit(4)];

    player_index_t player_index = color_to_player_index(COLOR_WHITE);
    coord_t zero_coord = coord_alg("a8");
    coord_t one_coord = coord_alg("b8");
    coord_t two_coord = coord_alg("c8");
    coord_t next_row = coord_alg("a7");
    coord_t last_row = coord_alg("e1");

    per_player_bitboard_set (bitboard, player_index, zero_coord, 4, 15);
    per_player_bitboard_set (bitboard, player_index, one_coord, 4, 11);
    per_player_bitboard_set (bitboard, player_index, two_coord, 4, 12);
    per_player_bitboard_set (bitboard, player_index, next_row, 4, 11);
    per_player_bitboard_set (bitboard, player_index, last_row, 4, 7);

    CHECK( per_player_bitboard_get (bitboard, player_index, zero_coord, 4) == 15 );
    CHECK( per_player_bitboard_get (bitboard, player_index, one_coord, 4) == 11 );
    CHECK( per_player_bitboard_get (bitboard, player_index, two_coord, 4) == 12 );
    CHECK( per_player_bitboard_get (bitboard, player_index, next_row, 4) == 11 );
    CHECK( per_player_bitboard_get (bitboard, player_index, last_row, 4) == 7 );
}
