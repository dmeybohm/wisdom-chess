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
    uint8_t players = 2;
    uint8_t rows = 2;
    memset (&bitboard, 0xff, sizeof(bitboard));

    player_index_t player_index = color_to_player_index(COLOR_WHITE);
    coord_t zero_coord = coord_alg("a8");
    coord_t one_coord = coord_alg("b8");
    coord_t two_coord = coord_alg("e8");
    coord_t next_row = coord_alg("d7");
    coord_t last_row = coord_alg("h7");

    // Test memset worked
    CHECK( compact_bitboard_get (bitboard, player_index, last_row, rows, players, 2) == 0x3 );

    compact_bitboard_set (bitboard, player_index, zero_coord, rows, players, 2, 3);
    compact_bitboard_set (bitboard, player_index, one_coord, rows, players, 2, 2);
    compact_bitboard_set (bitboard, player_index, two_coord, rows, players, 2, 1);
    compact_bitboard_set (bitboard, player_index, next_row, rows, players, 2, 2);
    compact_bitboard_set (bitboard, player_index, last_row, rows, players, 2, 3);

    CHECK( compact_bitboard_get (bitboard, player_index, zero_coord,rows, players, 2) == 3 );
    CHECK( compact_bitboard_get (bitboard, player_index, one_coord, rows, players, 2) == 2 );
    CHECK( compact_bitboard_get (bitboard, player_index, two_coord, rows, players, 2) == 1 );
    CHECK( compact_bitboard_get (bitboard, player_index, next_row,  rows, players, 2) == 2 );
    CHECK( compact_bitboard_get (bitboard, player_index, last_row,  rows, players, 2) == 3 );
}
