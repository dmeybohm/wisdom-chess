#include "catch.hpp"
#include "board_builder.hpp"

extern "C"
{
#include "bitboard.h"
}

TEST_CASE( "Bitboard size", "[bitboard]" )
{
    per_player_bitboard_t  bitboard[per_player_bitboard_total_units(4)];

    CHECK( sizeof(per_player_bitboard_t) == 8 );
    CHECK( sizeof(bitboard) == 64 ); // NR_ROWS * NR_COLUMNS * nr_bits_per_unit * NR_PLAYERS / 8
}

TEST_CASE( "Bitboard get/set", "[bitboard]" )
{
    per_player_bitboard_t  bitboard[per_player_bitboard_total_units(4)];
    memset (&bitboard, 0xff, sizeof(bitboard));

    auto row = GENERATE(0, 7);
    auto col = GENERATE(0, 7);
    auto value = GENERATE(0, 0xf);
    auto color = GENERATE(COLOR_WHITE, COLOR_BLACK);

    player_index_t player_index = color_to_player_index(color);
    coord_t coord = coord_create (row, col);

    // Test memset worked
    REQUIRE( per_player_bitboard_get (bitboard, player_index, coord, 4) == 0xf );

    per_player_bitboard_set (bitboard, player_index, coord, 4, value );

    REQUIRE( per_player_bitboard_get (bitboard, player_index, coord, 4) == value );
}
