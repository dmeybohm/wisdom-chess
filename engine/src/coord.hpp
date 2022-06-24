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
    constexpr auto is_valid_row (IntegerType row) -> bool
    {
        static_assert (std::is_integral<IntegerType>::value);
        return row >= 0 && row < Num_Rows;
    }

    template <class IntegerType> 
    constexpr auto is_valid_column (IntegerType col) -> bool
    {
        static_assert (std::is_integral<IntegerType>::value);
        return col >= 0 && col < Num_Columns;
    }

    template <class IntegerType> 
    constexpr auto next_row (IntegerType row, int direction) -> IntegerType
    {
        static_assert (std::is_integral<IntegerType>::value);
        return gsl::narrow_cast<IntegerType>(row + direction);
    }

    template <class T> 
    constexpr auto next_column (T col, int direction) -> T
    {
        static_assert (std::is_integral<T>::value);
        return gsl::narrow_cast<T>(col + direction);
    }

    constexpr auto make_coord (int row, int col) -> Coord
    {
        assert (is_valid_row (row) && is_valid_column (col));
        Coord result = { .row_and_col = gsl::narrow_cast<int8_t>(row << 3 | col) };
        return result;
    }

    constexpr Coord First_Coord = make_coord (0, 0);
    constexpr Coord End_Coord = { .row_and_col = Num_Squares };
    constexpr Coord No_En_Passant_Coord = First_Coord;

    // Return square index from zero to sixty-three, with a8 as 0 and h1 as 63.
    [[nodiscard]] constexpr auto coord_index (Coord coord) -> int
    {
        return coord.row_and_col;
    }

    // Return square index from zero to sixty-three, with a8 as 0 and h1 as 63.
    template <typename IntegerType = int8_t>
    [[nodiscard]] constexpr auto coord_index (IntegerType row, IntegerType col) -> int
    {
        static_assert (std::is_integral_v<IntegerType>);
        Coord coord = make_coord (row, col);
        return coord_index (coord);
    }

    // Make a coordinate from an index from 0-63.
    [[nodiscard]] constexpr auto make_coord_from_index (int index) -> Coord
    {
        assert (index >= 0 && index < Num_Squares);
        return { .row_and_col = gsl::narrow_cast<int8_t> (index) };
    }

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto Row (Coord pos) -> IntegerType
    {
        static_assert (std::is_integral<IntegerType>::value);
        return gsl::narrow_cast<IntegerType>(pos.row_and_col >> 3);
    }

    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto Column (Coord pos) -> IntegerType
    {
        static_assert (std::is_integral<IntegerType>::value);
        return gsl::narrow_cast<IntegerType>(pos.row_and_col & 0b111);
    }

    [[nodiscard]] constexpr auto next_coord (Coord coord, int direction) -> optional<Coord>
    {
        assert(direction == +1 || direction == -1);
        int index = coord_index (coord);
        index += direction;

        if (index < 0 || index >= Num_Squares)
            return {};

        return make_coord_from_index (index);
    }

    constexpr auto operator== (Coord first, Coord second) -> bool
    {
        return first.row_and_col == second.row_and_col;
    }

    constexpr bool operator!= (Coord first, Coord second)
    {
        return !operator== (first, second);
    }

    constexpr auto operator++ (Coord& coord) -> Coord&
    {
        coord.row_and_col++;
        return coord;
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

    class CoordIterator final
    {
    private:
        Coord my_coord {};

    public:
        CoordIterator ()
            : my_coord { First_Coord }
        {}

        explicit CoordIterator (Coord coord)
            : my_coord (coord)
        {}

        [[nodiscard]] auto begin () -> CoordIterator // NOLINT(readability-convert-member-functions-to-static)
        {
            return CoordIterator { First_Coord };
        }

        [[nodiscard]] auto end () -> CoordIterator // NOLINT(readability-convert-member-functions-to-static)
        {
            return CoordIterator { End_Coord };
        }

        auto operator* () const -> Coord
        {
            return my_coord;
        }

        auto operator++ () -> CoordIterator&
        {
            ++my_coord;
            return *this;
        }

        auto operator== (const CoordIterator &other) const -> bool
        {
            return other.my_coord == my_coord;
        }

        auto operator!= (const CoordIterator &other) const -> bool
        {
            return !(*this == other);
        }
    };

    class CoordParseError : public Error
    {
    public:
        explicit CoordParseError (string message) :
                Error (std::move (message))
        {}
    };
}

#endif // WISDOM_CHESS_COORD_H
