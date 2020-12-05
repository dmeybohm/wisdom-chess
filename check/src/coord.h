#ifndef EVOLVE_CHESS_COORD_H
#define EVOLVE_CHESS_COORD_H

#include "global.h"

#include <exception>
#include <string>

typedef struct coord
{
    int8_t row;
    int8_t col;
} coord_t;

///////////////////////////////////////////////

constexpr bool is_valid_row (int8_t row)
{
    return row >= 0 && row < NR_ROWS;
}

constexpr bool is_valid_column (int8_t col)
{
    return col >= 0 && col < NR_COLUMNS;
}

constexpr int8_t next_row (int8_t row, int8_t direction)
{
    return row + direction;
}

constexpr int8_t next_column (int8_t col, int8_t direction)
{
    return col + direction;
}

constexpr coord_t make_coord (int8_t row, int8_t col)
{
    assert (is_valid_row (row) && is_valid_column (col));
    coord_t result = { .row = row, .col = col };
    return result;
}

constexpr coord_t no_en_passant_coord = make_coord (0, 0);

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

constexpr char row_to_char (int row)
{
    return 8-row + '0';
}

constexpr char col_to_char (int col)
{
    return col + 'a';
}

/////////////////////////////////////////////////////////////////////

std::string to_string (coord_t coord);

coord_t coord_parse (const std::string &str);

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
