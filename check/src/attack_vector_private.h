#include "global.h"
#include "attack_vector.h"

//
// "Private" methods to the attack vector module.
//

//
// Attack vector bitboard functions.
//

static inline void add_other (struct attack_vector *attacks, player_index_t player_index,
                              uint8_t row, uint8_t col, int change)
{
    per_player_bitboard_add (attacks->other, player_index, coord_create (row, col), 4, change);
}

static inline void set_other (struct attack_vector *attacks, player_index_t player_index,
                              uint8_t row, uint8_t col, int change)
{
    per_player_bitboard_set (attacks->other, player_index, coord_create (row, col), 4, change);
}

static inline uint8_t get_other (const struct attack_vector *attacks, player_index_t player_index,
                                 uint8_t row, uint8_t col)
{
    return per_player_bitboard_get (attacks->other, player_index, coord_create (row, col), 4);
}

static inline void add_to_attack_vector (per_player_bitboard_t *vector, player_index_t player_index,
                                         uint8_t row, uint8_t col, int change)
{
    per_player_bitboard_add (vector, player_index, coord_create (row, col), 2, change);
}

static inline void set_attack_vector (per_player_bitboard_t *vector, player_index_t player_index,
                                      uint8_t row, uint8_t col, uint8_t value)
{
    per_player_bitboard_set (vector, player_index, coord_create (row, col), 2, value);
}

static inline uint8_t get_attack_vector (const per_player_bitboard_t *vector, player_index_t player_index,
                                         uint8_t row, uint8_t col)
{
    return per_player_bitboard_get (vector, player_index, coord_create (row, col), 2);
}

//////////////////////////////////////////////////////////////

//
// Relevant count bitboard functions.
//

static coord_t get_nw_to_se_bitboard_coord (coord_t coord)
{
    coord_t first_coord = first_nw_to_se_coord(coord);
    uint8_t row = ROW(first_coord);
    uint8_t col = COLUMN(first_coord);

    if (row >= 1)
    {
        col = row - 1;
        row = 1;
    }

    return coord_create (row, col);
}

static inline uint8_t get_nw_to_se_relevant_count (const struct attack_vector *attacks, coord_t coord)
{
    coord_t target = get_nw_to_se_bitboard_coord (coord);

    return bitboard_get (attacks->nw_to_se_relevant_count,  target,
                         2, NR_COLUMNS, 4);
}

static inline void add_to_nw_to_se_relevant_count (struct attack_vector *attacks, coord_t coord, int change)
{
    coord_t target = get_nw_to_se_bitboard_coord (coord);
    return bitboard_add (attacks->nw_to_se_relevant_count, target,
                         2, NR_COLUMNS, 4, change);
}

static coord_t get_nw_to_sw_bitboard_coord (coord_t coord)
{
    coord_t first_coord = first_ne_to_sw_coord(coord);
    uint8_t row = ROW(first_coord);
    uint8_t col = COLUMN(first_coord);

    if (row >= 1)
    {
        col = row - 1;
        row = 1;
    }

    coord_t target = coord_create (row, col);
    return target;
}

static inline uint8_t get_ne_to_sw_relevant_count (const struct attack_vector *attacks, coord_t coord)
{
    coord_t target = get_nw_to_sw_bitboard_coord (coord);

    return bitboard_get (attacks->ne_to_sw_relevant_count, target,
                         2, NR_COLUMNS, 4);
}

static inline void add_to_ne_to_sw_relevant_count (struct attack_vector *attacks, coord_t coord, int change)
{
    coord_t target = get_nw_to_se_bitboard_coord (coord);
    return bitboard_add (attacks->nw_to_se_relevant_count, target,
                         2, NR_COLUMNS, 4, change);
}

static coord_t get_horizontal_relevant_bitboard_coord (coord_t coord)
{
    uint8_t row = ROW(coord);

    // use row as col because bitboard is only a single row.
    return coord_create (0, row);
}

static inline uint8_t get_horizontal_relevant_count (const struct attack_vector *attacks, coord_t coord)
{
    coord_t bitboard_coord = get_horizontal_relevant_bitboard_coord (coord);

    return bitboard_get (attacks->horizontal_relevant_count, bitboard_coord,
                         1, NR_COLUMNS, 8);
}

static inline void add_to_horizontal_relevant_count (struct attack_vector *attacks, coord_t coord, int change)
{
    coord_t target = get_horizontal_relevant_bitboard_coord(coord);
    return bitboard_add (attacks->horizontal_relevant_count, target,
                         1, NR_COLUMNS, 8, change);
}

static coord_t get_vertical_relevant_bitboard_coord (coord_t coord)
{
    uint8_t col = COLUMN(coord);
    coord_t bitboard_coord = coord_create (0, col);
    return bitboard_coord;
}

static inline uint8_t get_vertical_relevant_count (const struct attack_vector *attacks, coord_t coord)
{
    coord_t bitboard_coord = get_vertical_relevant_bitboard_coord (coord);
    return bitboard_get (attacks->vertical_relevant_count, bitboard_coord,
                         1, NR_COLUMNS, 8);
}

static inline void add_to_vertical_relevant_count (struct attack_vector *attacks, coord_t coord, int change)
{
    coord_t target = get_vertical_relevant_bitboard_coord(coord);
    return bitboard_add (attacks->vertical_relevant_count, target,
                         1, NR_COLUMNS, 8, change);
}
