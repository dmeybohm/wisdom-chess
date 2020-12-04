#ifndef EVOLVE_CHESS_COORD_H
#define EVOLVE_CHESS_COORD_H

#include "global.h"

#include <cstdint>
#include <cctype>
#include <cstring>
#include <exception>
#include <string>

typedef struct coord
{
    int8_t row;
    int8_t col;
} coord_t;

///////////////////////////////////////////////

constexpr bool VALID (int8_t row_or_col)
{
    return ((row_or_col) >= 0 && (row_or_col) < 8);
}

constexpr int8_t NEXT (int8_t row_or_col, int8_t direction)
{
    return row_or_col + direction;
}

constexpr bool INVALID (int8_t row_or_col)
{
    return !VALID(row_or_col);
}

constexpr coord_t coord_create (int8_t row, int8_t col)
{
    assert (VALID(row) && VALID(col));
    coord_t result = { .row = row, .col = col };
    return result;
}

constexpr coord_t no_en_passant_coord = coord_create (0, 0);

///////////////////////////////////////////////

constexpr int8_t ROW (coord_t pos)
{
    return pos.row;
}

constexpr int8_t COLUMN (coord_t pos)
{
	return pos.col;
}

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

static inline int8_t char_to_row (char chr)
{
    return 8 - (tolower(chr) - '0');
}

static inline int8_t char_to_col (char chr)
{
    return tolower(chr) - 'a';
}

/////////////////////////////////////////////////////////////////////

coord_t coord_parse (std::string_view str);

/////////////////////////////////////////////////////////////////////

class coord_parse_exception : public std::exception
{
private:
    const char *message;

public:
    explicit coord_parse_exception (const char *message) : message { message } {}
    [[nodiscard]] const char *what() const noexcept override { return this->message; }
};

#endif // EVOLVE_CHESS_COORD_H
