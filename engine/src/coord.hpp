#ifndef WISDOM_CHESS_COORD_H
#define WISDOM_CHESS_COORD_H

#include "global.hpp"

namespace wisdom
{
    struct coord
    {
        int8_t row : 4;
        int8_t col : 4;
    };

    using Coord = struct coord;

    static_assert(std::is_trivial<Coord>::value);

    ///////////////////////////////////////////////

    constexpr bool is_valid_row (int row)
    {
        return row >= 0 && row < Num_Rows;
    }

    constexpr bool is_valid_column (int col)
    {
        return col >= 0 && col < Num_Columns;
    }

    constexpr int next_row (int row, int direction)
    {
        return row + direction;
    }

    constexpr int next_column (int col, int direction)
    {
        return col + direction;
    }

    constexpr Coord make_coord (int row, int col)
    {
        assert (is_valid_row (row) && is_valid_column (col));
        int8_t row8 = static_cast<int8_t>(row);
        int8_t col8 = static_cast<int8_t>(col);
        Coord result = { .row = row8, .col = col8 };
        return result;
    }

    constexpr Coord No_En_Passant_Coord = make_coord (0, 0);

    ///////////////////////////////////////////////

    constexpr int Row (Coord pos)
    {
        return pos.row;
    }

    constexpr int Column (Coord pos)
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

    static inline int char_to_row (char chr)
    {
        return 8 - (tolower (chr) - '0');
    }

    static inline int char_to_col (char chr)
    {
        return tolower (chr) - 'a';
    }

    constexpr char row_to_char (int row)
    {
        assert (is_valid_row (row));
        return 8 - row + '0';
    }

    constexpr char col_to_char (int col)
    {
        assert (is_valid_column (col));
        return col + 'a';
    }

    /////////////////////////////////////////////////////////////////////

    std::string to_string (Coord coord);

    Coord coord_parse (const std::string &str);

    /////////////////////////////////////////////////////////////////////

    class CoordParseError : public Error
    {
    public:
        explicit CoordParseError (std::string message) :
                Error (std::move (message))
        {}
    };
}

#endif // WISDOM_CHESS_COORD_H
