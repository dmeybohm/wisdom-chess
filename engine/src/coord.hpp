#ifndef WISDOM_CHESS_COORD_H
#define WISDOM_CHESS_COORD_H

#include "global.hpp"

namespace wisdom
{
    struct coord
    {
        int8_t row_and_col;
    };

    using Coord = struct coord;

    static_assert(std::is_trivial_v<Coord>);

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
        Coord result = { .row_and_col = gsl::narrow_cast<int8_t>(row << 4 | col) };
        return result;
    }

    constexpr Coord No_En_Passant_Coord = make_coord (0, 0);

    constexpr int8_t Row (Coord pos)
    {
        return gsl::narrow_cast<int8_t>(pos.row_and_col >> 4);
    }

    constexpr int8_t Column (Coord pos)
    {
        return gsl::narrow_cast<int8_t>(pos.row_and_col & 0xf);
    }

    constexpr bool coord_equals (Coord a, Coord b)
    {
        return a.row_and_col == b.row_and_col;
    }

    constexpr bool operator== (Coord a, Coord b)
    {
        return coord_equals (a, b);
    }

    constexpr bool operator!= (Coord a, Coord b)
    {
        return !coord_equals (a, b);
    }

    static inline int char_to_row (char chr)
    {
        return 8 - (tolower (chr) - '0');
    }

    static inline int char_to_col (char chr)
    {
        return tolower (chr) - 'a';
    }

    constexpr char row_to_char (int8_t row)
    {
        assert (is_valid_row (row));
        return gsl::narrow<char> (8 - row + '0');
    }

    constexpr char col_to_char (int8_t col)
    {
        assert (is_valid_column (col));
        return gsl::narrow<char> (col + 'a');
    }

    auto to_string (Coord coord) -> string;

    auto coord_parse (const string &str) -> Coord;

    class CoordParseError : public Error
    {
    public:
        explicit CoordParseError (string message) :
                Error (std::move (message))
        {}
    };
}

#endif // WISDOM_CHESS_COORD_H
