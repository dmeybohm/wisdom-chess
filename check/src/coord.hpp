#ifndef WISDOM_CHESS_COORD_H
#define WISDOM_CHESS_COORD_H

#include "global.hpp"

#include <exception>
#include <string>

namespace wisdom
{
    struct coord
    {
        int8_t row;
        int8_t col;
    };

    using Coord = struct coord;

    ///////////////////////////////////////////////

    constexpr bool is_valid_row (int8_t row)
    {
        return row >= 0 && row < Num_Rows;
    }

    constexpr bool is_valid_column (int8_t col)
    {
        return col >= 0 && col < Num_Columns;
    }

    constexpr int8_t next_row (int8_t row, int8_t direction)
    {
        return row + direction;
    }

    constexpr int8_t next_column (int8_t col, int8_t direction)
    {
        return col + direction;
    }

    constexpr Coord make_coord (int8_t row, int8_t col)
    {
        assert (is_valid_row (row) && is_valid_column (col));
        Coord result = { .row = row, .col = col };
        return result;
    }

    constexpr Coord No_En_Passant_Coord = make_coord (0, 0);

    ///////////////////////////////////////////////

    constexpr int8_t ROW (Coord pos)
    {
        return pos.row;
    }

    constexpr int8_t COLUMN (Coord pos)
    {
        return pos.col;
    }

    constexpr bool coord_equals (Coord a, Coord b)
    {
        return a.row == b.row && a.col == b.col;
    }

    constexpr bool operator== (Coord a, Coord b)
    {
        return coord_equals (a, b);
    }

    constexpr bool operator!= (Coord a, Coord b)
    {
        return !coord_equals (a, b);
    }

    static inline int8_t char_to_row (char chr)
    {
        return 8 - (tolower (chr) - '0');
    }

    static inline int8_t char_to_col (char chr)
    {
        return tolower (chr) - 'a';
    }

    constexpr char row_to_char (int row)
    {
        return 8 - row + '0';
    }

    constexpr char col_to_char (int col)
    {
        return col + 'a';
    }

    /////////////////////////////////////////////////////////////////////

    std::string to_string (Coord coord);

    Coord coord_parse (const std::string &str);

    /////////////////////////////////////////////////////////////////////

    class CoordParseException : public std::exception
    {
    private:
        const char *message;

    public:
        explicit CoordParseException (const char *message) : message { message }
        {}

        [[nodiscard]] const char *what () const noexcept override
        { return this->message; }
    };

}
#endif // WISDOM_CHESS_COORD_H
