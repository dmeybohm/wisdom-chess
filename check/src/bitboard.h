#ifndef WIZDUMB_BITBOARD_H
#define WIZDUMB_BITBOARD_H

#include "global.h"
#include "coord.h"
#include "piece.h"

typedef struct bitboard
{
    uint64_t bits;
} bitboard_t;

typedef struct per_player_bitboard
{
    uint64_t bits;
} per_player_bitboard_t;

////////////////////////////////////////////////////////////

#define bitboard_units(_nr_rows, _nr_cols, _bits) \
    (((_nr_rows) * (_nr_cols) * (_bits)) / 64)

#define bitboard_index(_row, _col, _bits_per_unit) \
    ( ( ( (_row << 3U) + (_col) ) * (_bits_per_unit) ) / 64U)

#define bitboard_offset(_row, _col, _bits_per_unit) \
    ( ( ( (_row << 3U) + (_col) ) * (_bits_per_unit) ) % 64U)

#define bitboard_mask(_bits_per_unit) \
    ((uint64_t)((1ULL << (_bits_per_unit)) - 1ULL))

////////////////////////////////////////////////////////////

static inline uint8_t bitboard_get (const bitboard_t *bitboard,
                                    coord_t coord, uint8_t nr_rows, uint8_t nr_cols,
                                    uint8_t bits_per_unit)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    uint64_t total_units = bitboard_units (nr_rows, nr_cols, bits_per_unit);
    uint64_t index = bitboard_index (row, col,  bits_per_unit);
    uint64_t offset = bitboard_offset (row, col,  bits_per_unit);
    uint64_t mask = bitboard_mask (bits_per_unit);

    assert (index < total_units);
    return (bitboard[index].bits >> offset) & mask;
}

static inline void bitboard_set (bitboard_t *bitboard, coord_t coord,
                                 uint8_t nr_rows, uint8_t nr_cols,
                                 uint8_t bits_per_unit, uint8_t value)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    uint64_t total_units = bitboard_units (nr_rows, nr_cols, bits_per_unit);
    uint64_t index = bitboard_index (row, col, bits_per_unit);
    uint64_t offset = bitboard_offset (row, col, bits_per_unit);
    uint64_t mask = bitboard_mask(bits_per_unit);
    uint64_t value64 = value;

    assert (index < total_units);
    assert (value < (1 << bits_per_unit));

    uint64_t bits = bitboard[index].bits;
    bits &= ~(mask << offset);
    bits |= (value64 << offset);
    bitboard[index].bits = bits;
}

static inline void bitboard_add (bitboard_t *bitboard, coord_t coord,
                                 uint8_t nr_rows, uint8_t nr_cols,
                                 uint8_t bits_per_unit, int8_t value)
{
    int new_value = value + bitboard_get (bitboard, coord, nr_rows, nr_cols, bits_per_unit);
    assert (new_value >= 0 && value <= 255);
    bitboard_set (bitboard, coord, nr_rows, nr_cols, bits_per_unit, new_value);
}

////////////////////////////////////////////////////////////

#define per_player_bitboard_units(_bits) \
    ((NR_ROWS * NR_COLUMNS * (_bits) * NR_PLAYERS) / 64U)

#define per_player_bitboard_index(_row, _col, _player_index, _bits_per_unit) \
    ( (( (_row << 4U) + ( (_col) << 1U) + (_player_index) ) * (_bits_per_unit)) / 64U)

#define per_player_bitboard_offset(_row, _col, _player_index, _bits_per_unit) \
    ( (( (_row << 4U) + ( (_col) << 1U) + (_player_index) ) * (_bits_per_unit)) % 64U)

#define per_player_bitboard_mask(_bits_per_unit) \
    ((uint64_t)((1ULL << (_bits_per_unit)) - 1ULL))

////////////////////////////////////////////////////////////

static inline uint8_t per_player_bitboard_get (const per_player_bitboard_t *bitboard,
                                               player_index_t player_index,
                                               coord_t coord, uint8_t bits_per_unit)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    uint64_t total_units = per_player_bitboard_units(bits_per_unit);
    uint64_t index = per_player_bitboard_index (row, col, player_index.index, bits_per_unit);
    uint64_t offset = per_player_bitboard_offset (row, col, player_index.index, bits_per_unit);
    uint64_t mask = per_player_bitboard_mask (bits_per_unit);

    assert (index < total_units);
    return (bitboard[index].bits >> offset) & mask;
}

static inline void per_player_bitboard_set (per_player_bitboard_t *bitboard,
                                            player_index_t player_index, coord_t coord,
                                            uint8_t bits_per_unit, uint8_t value)
{
    uint8_t row = ROW(coord);
    uint8_t col = COLUMN(coord);

    uint64_t total_units = per_player_bitboard_units(bits_per_unit);
    uint64_t index = per_player_bitboard_index (row, col, player_index.index, bits_per_unit);
    uint64_t offset = per_player_bitboard_offset (row, col, player_index.index, bits_per_unit);
    uint64_t mask = per_player_bitboard_mask(bits_per_unit);
    uint64_t value64 = value;

    assert (index < total_units);
    assert (value < (1 << bits_per_unit));

    uint64_t bits = bitboard[index].bits;
    bits &= ~(mask << offset);
    bits |= (value64 << offset);
    bitboard[index].bits = bits;
}

static inline void per_player_bitboard_add (per_player_bitboard_t *bitboard,
                                            player_index_t player_index, coord_t coord,
                                            uint8_t bits_per_unit, int8_t value)
{
    int new_value = value + per_player_bitboard_get (bitboard, player_index, coord, bits_per_unit);
    assert (new_value >= 0 && value <= 255);
    per_player_bitboard_set (bitboard, player_index, coord, bits_per_unit, new_value);
}

#endif //WIZDUMB_BITBOARD_H
