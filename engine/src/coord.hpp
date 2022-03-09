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

    template <typename T>
    constexpr auto is_valid_row (T row) -> bool
    {
        return row >= 0 && row < Num_Rows;
    }

    template <typename T>
    constexpr auto is_valid_column (T col) -> bool
    {
        return col >= 0 && col < Num_Columns;
    }

    template <class T>
    constexpr auto next_row (T row, int direction) -> T
    {
        return gsl::narrow_cast<T>(row + direction);
    }

    template <class T>
    constexpr auto next_column (T col, int direction) -> T
    {
        return gsl::narrow_cast<T>(col + direction);
    }

    constexpr Coord make_coord (int row, int col)
    {
        assert (is_valid_row (row) && is_valid_column (col));
        Coord result = { .row_and_col = gsl::narrow_cast<int8_t>(row << 4 | col) };
        return result;
    }

    constexpr Coord No_En_Passant_Coord = make_coord (0, 0);

    template <class T = int8_t>
    constexpr auto Row (Coord pos) -> T
    {
        return gsl::narrow_cast<T>(pos.row_and_col >> 4);
    }

    template <class T = int8_t>
    constexpr auto Column (Coord pos) -> T
    {
        return gsl::narrow_cast<T>(pos.row_and_col & 0xf);
    }

    constexpr auto operator== (Coord first, Coord second) -> bool
    {
        return first.row_and_col == second.row_and_col;
    }

    constexpr bool operator!= (Coord first, Coord second)
    {
        return !operator== (first, second);
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

    auto coord_parse (const string& str) -> Coord;

    class CoordParseError : public Error
    {
    public:
        explicit CoordParseError (string message) :
                Error (std::move (message))
        {}
    };
}

#endif // WISDOM_CHESS_COORD_H
