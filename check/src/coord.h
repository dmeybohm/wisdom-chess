#ifndef EVOLVE_CHESS_COORD_H
#define EVOLVE_CHESS_COORD_H

#include "global.h"

#include <cstdint>
#include <cctype>
#include <cstring>
#include <exception>

// lower three bits are the column, upper three are the row
typedef struct coord
{
    uint8_t row : 3;
    uint8_t col : 3;
} coord_t;

constexpr uint8_t ROW (coord_t pos)
{
    return pos.row;
}

constexpr uint8_t COLUMN (coord_t pos)
{
	return pos.col;
}

constexpr coord_t coord_create (uint8_t row, uint8_t col)
{
    coord_t result = { .row = row, .col = col };
    return result;
}

constexpr coord_t no_en_passant_coord = coord_create (0, 0);

constexpr bool coord_equals (coord_t a, coord_t b)
{
    return a.row == b.row && a.col == b.col;
}

constexpr bool operator == (coord_t a, coord_t b)
{
    return coord_equals (a, b);
}

constexpr bool operator != (coord_t a, coord_t b)
{
    return !coord_equals (a, b);
}

static inline uint8_t char_to_row (char chr)
{
    return 8 - (tolower(chr) - '0');
}

static inline uint8_t char_to_col (char chr)
{
    return tolower(chr) - 'a';
}

/////////////////////////////////////////////////////////////////////

coord_t coord_parse (const char *str);

/////////////////////////////////////////////////////////////////////

class coord_parse_exception : public std::exception
{
    const char *message;

public:
    explicit coord_parse_exception (const char *message) : message { message } {}
    [[nodiscard]] const char *what() const noexcept override { return this->message; }
};

#endif // EVOLVE_CHESS_COORD_H
