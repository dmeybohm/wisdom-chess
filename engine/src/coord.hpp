#ifndef WISDOM_CHESS_COORD_H
#define WISDOM_CHESS_COORD_H

#include "global.hpp"
#include "piece.hpp"

namespace wisdom
{
    struct Coord
    {
        int8_t row_and_col;
    };

    static_assert(std::is_trivial_v<Coord>);

    template <class IntegerType>
        requires std::is_integral_v<IntegerType>
    constexpr auto is_valid_row (IntegerType row) -> bool
    {
        return row >= 0 && row < Num_Rows;
    }

    template <class IntegerType>
        requires std::is_integral_v<IntegerType>
    constexpr auto is_valid_column (IntegerType col) -> bool
    {
        return col >= 0 && col < Num_Columns;
    }

    template <class IntegerType>
        requires std::is_integral_v<IntegerType>
    constexpr auto next_row (IntegerType row, int direction) -> IntegerType
    {
        return gsl::narrow_cast<IntegerType>(row + direction);
    }

    template <class T>
        requires std::is_integral_v<T>
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

    constexpr Coord First_Coord = make_coord (0, 0);
    constexpr Coord No_En_Passant_Coord = First_Coord;

    template <class IntegerType = int8_t>
        requires std::is_integral_v<IntegerType>
    constexpr auto Row (Coord pos) -> IntegerType
    {
        return gsl::narrow_cast<IntegerType>(pos.row_and_col >> 4);
    }

    template <class IntegerType = int8_t>
        requires std::is_integral_v<IntegerType>
    constexpr auto Column (Coord pos) -> IntegerType
    {
        return gsl::narrow_cast<IntegerType>(pos.row_and_col & 0xf);
    }

    constexpr auto next_coord (Coord coord, int direction) -> optional<Coord>
    {
        assert(direction == +1 || direction == -1);
        int row = Row<int> (coord);
        int col = Column<int> (coord);
        col += direction;

        if (!is_valid_column (col))
        {
            col = 0;
            row += direction;
            if (!is_valid_row (row))
                return {};
        }

        return make_coord (row, col);
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

    auto operator<< (std::ostream& ostream, Coord coord) -> std::ostream&;

    class CoordParseError : public Error
    {
    public:
        explicit CoordParseError (string message) :
                Error (std::move (message))
        {}
    };
}

#endif // WISDOM_CHESS_COORD_H