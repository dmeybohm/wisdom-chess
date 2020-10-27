extern "C"
{
#include "compact_bitboard.h"
}

#include "catch.hpp"
#include "board_builder.hpp"

TEST_CASE( "Compact bitboard size", "[compact-bitboard]")
{
    compact_bitboard_t bitboard[compact_bitboard_total_units (2, 2, 2)];

    CHECK( sizeof(compact_bitboard_t) == 8 );
    CHECK( sizeof(bitboard) == 8 ); // NR_DIAGONAL_ROWS * NR_COLUMNS * nr_bits_per_unit * NR_PLAYERS * 8
}

TEST_CASE( "Compact bitboard get/set", "[compact-bitboard]" )
{
    compact_bitboard_t bitboard[compact_bitboard_total_units (2, 2, 2)];
    memset (&bitboard, 0xff, sizeof(bitboard));

    auto row = GENERATE(0, 1);
    auto col = GENERATE(0, 7);
    auto value = GENERATE(0, 0x3);
    auto color = GENERATE(COLOR_WHITE, COLOR_BLACK);

    player_index_t player_index = color_to_player_index(color);
    coord_t coord = coord_create (row, col);

    // Test memset worked
    REQUIRE( compact_bitboard_get (bitboard, player_index, coord, 2, 2, 2) == 0x3 );

    compact_bitboard_set (bitboard, player_index, coord, 2, 2, 2, value );

    REQUIRE( compact_bitboard_get (bitboard, player_index, coord, 2, 2, 2) == value );
}
