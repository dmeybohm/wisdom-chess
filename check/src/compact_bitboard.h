#ifndef WIZDUMB_COMPACT_BITBOARD_H
#define WIZDUMB_COMPACT_BITBOARD_H

#include "bitboard.h"

typedef struct compact_bitboard
{
    uint64_t bits;
} compact_bitboard_t;

////////////////////////////////////////////////////////////

#define compact_bitboard_total_units(_rows, _nr_players, _bits) \
        bitboard_units_for_bits((_rows), NR_COLUMNS, (_nr_players), (_bits))

#define compact_bitboard_total_units_for_bits(_rows, _bits) \
        bitboard_units_for_bits(NR_DIAGONAL_ROWS, NR_COLUMNS, (_bits))

#define compact_bitboard_index(_row, _col, _player_index, _bits_per_unit) \
    ( (( (_row << 4U) + ( (_col) << 1U) + (_player_index) ) * (_bits_per_unit)) / 64U)

#define compact_bitboard_offset(_row, _col, _player_index, _bits_per_unit) \
    ( (( (_row << 4U) + ( (_col) << 1U) + (_player_index) ) * (_bits_per_unit)) % 64U)

#define compact_bitboard_mask(_bits_per_unit) \
    ((uint64_t)((1ULL << (_bits_per_unit)) - 1ULL))

////////////////////////////////////////////////////////////

static inline uint8_t compact_bitboard_get (const compact_bitboard_t *bitboard,
                                            player_index_t player_index, coord_t coord,
                                            uint8_t nr_rows, uint8_t nr_players,
                                            uint8_t bits_per_unit)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    uint64_t total_units = compact_bitboard_total_units (nr_rows, nr_players, bits_per_unit);
    uint64_t index = compact_bitboard_index (row, col, player_index.index, bits_per_unit);
    uint64_t offset = compact_bitboard_offset (row, col, player_index.index, bits_per_unit);
    uint64_t mask = compact_bitboard_mask (bits_per_unit);

    assert (index < total_units);
    return (bitboard[index].bits >> offset) & mask;
}

static inline void compact_bitboard_set (compact_bitboard_t *bitboard,
                                         player_index_t player_index, coord_t coord,
                                         uint8_t nr_rows, uint8_t nr_players,
                                         uint8_t bits_per_unit, uint8_t value)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    uint64_t total_units = compact_bitboard_total_units (nr_rows, nr_players, bits_per_unit);
    uint64_t index = compact_bitboard_index (row, col, player_index.index, bits_per_unit);
    uint64_t offset = compact_bitboard_offset (row, col, player_index.index, bits_per_unit);
    uint64_t mask = compact_bitboard_mask(bits_per_unit);
    uint64_t value64 = value;

    assert (index < total_units);
    assert (value < (1 << bits_per_unit));

    uint64_t bits = bitboard[index].bits;
    bits &= ~(mask << offset);
    bits |= (value64 << offset);
    bitboard[index].bits = bits;
}

static inline void compact_bitboard_add (compact_bitboard_t *bitboard,
                                         player_index_t player_index, coord_t coord,
                                         uint8_t nr_rows, uint8_t nr_players,
                                         uint8_t bits_per_unit, uint8_t value)
{
    value += compact_bitboard_get (bitboard, player_index, coord, nr_rows, nr_players, bits_per_unit);
    compact_bitboard_set (bitboard, player_index, coord, nr_rows, nr_players, bits_per_unit, value);
}

#endif //WIZDUMB_COMPACT_BITBOARD_H
