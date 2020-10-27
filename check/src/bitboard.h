#ifndef WIZDUMB_BITBOARD_H
#define WIZDUMB_BITBOARD_H

#include "global.h"
#include "coord.h"
#include "piece.h"

typedef struct per_player_bitboard
{
    uint64_t bits;
} per_player_bitboard_t;

////////////////////////////////////////////////////////////

#define bits_per_unit(_bits) \
    (((NR_ROWS * NR_COLUMNS) * 64U / ((_bits) * 2U)) / 64)

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

    uint64_t total_units = bits_per_unit(bits_per_unit);
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

    uint64_t total_units = bits_per_unit(bits_per_unit);
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

#endif //WIZDUMB_BITBOARD_H
