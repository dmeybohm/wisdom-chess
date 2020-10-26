#ifndef EVOLVE_CHESS_COORD_H
#define EVOLVE_CHESS_COORD_H

#include <stdint.h>
#include <ctype.h>
#include <string.h>

// lower three bits are the column, upper three are the row
typedef uint8_t coord_t;

// three bits for the row and column each
#define COORD_MASK   (0x7U)
#define COORD_SHIFT  (3U)

static inline uint8_t ROW (coord_t pos)
{
	uint8_t raw_pos = pos >> COORD_SHIFT;
	return raw_pos & COORD_MASK;
}

static inline uint8_t COLUMN (coord_t pos)
{
	return ((uint8_t) pos) & COORD_MASK;
}

static inline coord_t coord_create (uint8_t row, uint8_t col)
{
	uint8_t raw_row = row << COORD_SHIFT;
	uint8_t raw_col = col & COORD_MASK;
	return raw_row | raw_col;
}

static inline unsigned char char_to_row (char chr)
{
    return 8 - (tolower (chr) - '0');
}

static inline unsigned char char_to_col (char chr)
{
    return tolower (chr) - 'a';
}

#endif // EVOLVE_CHESS_COORD_H
