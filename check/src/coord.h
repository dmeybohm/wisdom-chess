#ifndef EVOLVE_CHESS_COORD_H
#define EVOLVE_CHESS_COORD_H

#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "global.h"

// lower three bits are the column, upper three are the row
typedef struct coord
{
    uint8_t _row : 3;
    uint8_t _col : 3;
} coord_t;

constexpr uint8_t ROW (coord_t pos)
{
    return pos._row;
}

constexpr uint8_t COLUMN (coord_t pos)
{
	return pos._col;
}

constexpr coord_t coord_create (uint8_t row, uint8_t col)
{
    coord_t result = { ._row = row, ._col = col };
    return result;
}

constexpr bool coord_equals (coord_t a, coord_t b)
{
    return a._row == b._row && a._col == b._col;
}

static inline uint8_t char_to_row (char chr)
{
    return 8 - (tolower(chr) - '0');
}

static inline uint8_t char_to_col (char chr)
{
    return tolower(chr) - 'a';
}

#endif // EVOLVE_CHESS_COORD_H
