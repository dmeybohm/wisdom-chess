#pragma once

#include "global.hpp"

namespace wisdom
{
    template <typename IntegerType>
    constexpr auto isValidRow (IntegerType row) -> bool
    {
        static_assert (std::is_integral_v<IntegerType>);
        return row >= 0 && row < Num_Rows;
    }

    template <typename IntegerType>
    constexpr auto isValidColumn (IntegerType col) -> bool
    {
        static_assert (std::is_integral_v<IntegerType>);
        return col >= 0 && col < Num_Columns;
    }

    struct Coord
    {
        int8_t row_and_col;

        // Make a coordinate from an index from 0-63.
        [[nodiscard]] static constexpr auto 
        fromIndex (int index) 
            -> Coord
        {
            assert (index >= 0 && index < Num_Squares);
            return { .row_and_col = gsl::narrow_cast<int8_t> (index) };
        }

        [[nodiscard]] static constexpr auto 
        make (int row, int col) 
            -> Coord
        {
            assert (isValidRow (row) && isValidColumn (col));
            Coord result = { .row_and_col = gsl::narrow_cast<int8_t> (row << 3 | col) };
            return result;
        }

        // Return square index from zero to sixty-three, with a8 as 0 and h1 as 63.
        template <typename IntegerType = int>
        [[nodiscard]] constexpr auto 
        index() 
            -> IntegerType
        {
            return gsl::narrow_cast<IntegerType> (row_and_col);
        }

        template <typename IntegerType = int8_t>
        [[nodiscard]] constexpr auto 
        row() 
            -> IntegerType
        {
            static_assert (std::is_integral_v<IntegerType>);
            return gsl::narrow_cast<IntegerType> (row_and_col >> 3);
        }

        template <typename IntegerType = int8_t>
        [[nodiscard]] constexpr auto 
        column() 
            -> IntegerType
        {
            static_assert (std::is_integral_v<IntegerType>);
            return gsl::narrow_cast<IntegerType> (row_and_col & 0b111);
        }
    };
    static_assert (std::is_trivial_v<Coord>);

    template <typename IntegerType>
    constexpr auto nextRow (IntegerType row, int direction) -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return gsl::narrow_cast<IntegerType> (row + direction);
    }

    template <typename T>
    constexpr auto nextColumn (T col, int direction) -> T
    {
        static_assert (std::is_integral_v<T>);
        return gsl::narrow_cast<T> (col + direction);
    }

    constexpr auto makeCoord (int row, int col) -> Coord
    {
        return Coord::make (row, col);
    }

    constexpr Coord First_Coord = makeCoord (0, 0);
    constexpr Coord End_Coord = { .row_and_col = Num_Squares };

    template <typename IntegerType = int8_t>
    [[nodiscard]] constexpr auto coordRow (Coord pos) -> IntegerType
    {
        return pos.row<IntegerType>();
    }

    template <typename IntegerType = int8_t>
    [[nodiscard]] constexpr auto coordColumn (Coord pos) -> IntegerType
    {
        return pos.column<IntegerType>();
    }

    [[nodiscard]] constexpr auto nextCoord (Coord coord, int direction) -> optional<Coord>
    {
        Expects (direction == +1 || direction == -1);
        int index = coord.index();
        index += direction;

        if (index < 0 || index >= Num_Squares)
            return {};

        return Coord::fromIndex (index);
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

    static inline int charToRow (char chr)
    {
        return 8 - (tolower (chr) - '0');
    }

    static inline int charToCol (char chr)
    {
        return tolower (chr) - 'a';
    }

    constexpr char rowToChar (int8_t row)
    {
        assert (isValidRow (row));
        return gsl::narrow<char> (8 - row + '0');
    }

    constexpr char colToChar (int8_t col)
    {
        assert (isValidColumn (col));
        return gsl::narrow<char> (col + 'a');
    }

    auto asString (Coord coord) -> string;

    auto coordParse (const string& str) -> Coord;

    auto operator<< (std::ostream& ostream, Coord coord) -> std::ostream&;

    class CoordIterator
    {
    public:
        using difference_type = int;
        using value_type = Coord;
        using reference = Coord&;
        using iterator_category = std::forward_iterator_tag;

        CoordIterator()
            : my_coord { First_Coord }
        {}

        explicit CoordIterator (Coord coord)
            : my_coord (coord)
        {}

        [[nodiscard]] auto begin() -> CoordIterator // NOLINT(readability-convert-member-functions-to-static)
        {
            return CoordIterator { First_Coord };
        }

        [[nodiscard]] auto end() -> CoordIterator // NOLINT(readability-convert-member-functions-to-static)
        {
            return CoordIterator { End_Coord };
        }

        auto operator*() const -> Coord
        {
            return my_coord;
        }

        auto operator++() -> CoordIterator&
        {
            ++my_coord;
            return *this;
        }

        auto operator== (const CoordIterator& other) const -> bool
        {
            return other.my_coord == my_coord;
        }

        auto operator!= (const CoordIterator& other) const -> bool
        {
            return !(*this == other);
        }

    private:
        Coord my_coord {};
    };

    class CoordParseError : public Error
    {
    public:
        explicit CoordParseError (string message)
            : Error (std::move (message))
        {
        }
    };
}
