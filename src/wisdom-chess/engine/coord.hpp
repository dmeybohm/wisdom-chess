#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/str.hpp"

namespace wisdom
{
    template <typename IntegerType>
    [[nodiscard]] constexpr auto
    isValidRow (IntegerType row) noexcept
        -> bool
    {
        static_assert (std::is_integral_v<IntegerType>);
        return row >= 0 && row < Num_Rows;
    }

    template <typename IntegerType>
    [[nodiscard]] constexpr auto
    isValidColumn (IntegerType col) noexcept
        -> bool
    {
        static_assert (std::is_integral_v<IntegerType>);
        return col >= 0 && col < Num_Columns;
    }

    struct Coord
    {
        int8_t row_and_col;

        // Make a coordinate from an index from 0-63.
        [[nodiscard]] static constexpr auto
        fromIndex (int index) noexcept
            -> Coord
        {
            ASSERT( index >= 0 && index < Num_Squares );
            return { .row_and_col = narrow_debug<int8_t> (index) };
        }

        [[nodiscard]] static constexpr auto
        make (int row, int col) noexcept
            -> Coord
        {
            ASSERT( isValidRow (row) && isValidColumn (col) );
            Coord result = { .row_and_col = narrow_debug<int8_t> (row << 3 | col) };
            return result;
        }

        // Return square index from zero to sixty-three, with a8 as 0 and h1 as 63.
        template <typename IntegerType = int>
        [[nodiscard]] constexpr auto
        index() const noexcept
            -> IntegerType
        {
            return narrow_debug<IntegerType> (row_and_col);
        }

        template <typename IntegerType = int8_t>
        [[nodiscard]] constexpr auto
        row() const noexcept
            -> IntegerType
        {
            static_assert (std::is_integral_v<IntegerType>);
            return narrow_debug<IntegerType> (row_and_col >> 3);
        }

        template <typename IntegerType = int8_t>
        [[nodiscard]] constexpr auto
        column() const noexcept
            -> IntegerType
        {
            static_assert (std::is_integral_v<IntegerType>);
            return narrow_debug<IntegerType> (row_and_col & 0b111);
        }
    };
    static_assert (std::is_trivial_v<Coord>);

    template <typename IntegerType>
    [[nodiscard]] constexpr auto
    nextRow (IntegerType row, int direction) noexcept
        -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        return narrow_debug<IntegerType> (row + direction);
    }

    template <typename T>
    [[nodiscard]] constexpr auto
    nextColumn (T col, int direction) noexcept
        -> T
    {
        static_assert (std::is_integral_v<T>);
        return narrow_debug<T> (col + direction);
    }

    [[nodiscard]] constexpr auto
    makeCoord (int row, int col) noexcept
        -> Coord
    {
        return Coord::make (row, col);
    }

    constexpr Coord First_Coord = makeCoord (0, 0);
    constexpr Coord End_Coord = { .row_and_col = Num_Squares };

    template <typename IntegerType = int8_t>
    [[nodiscard]] constexpr auto
    coordRow (Coord pos) noexcept
        -> IntegerType
    {
        return pos.row<IntegerType>();
    }

    template <typename IntegerType = int8_t>
    [[nodiscard]] constexpr auto
    coordColumn (Coord pos) noexcept
        -> IntegerType
    {
        return pos.column<IntegerType>();
    }

    [[nodiscard]] constexpr auto
    nextCoord (Coord coord) noexcept
        -> optional<Coord>
    {
        int index = coord.index();
        index += 1;

        if (index >= Num_Squares)
            return {};

        return Coord::fromIndex (index);
    }

    [[nodiscard]] constexpr auto
    operator== (Coord first, Coord second) noexcept
        -> bool
    {
        return first.row_and_col == second.row_and_col;
    }

    [[nodiscard]] constexpr auto
    operator!= (Coord first, Coord second) noexcept
        -> bool
    {
        return !operator== (first, second);
    }

    constexpr auto
    operator++ (Coord& coord) noexcept
        -> Coord&
    {
        coord.row_and_col++;
        return coord;
    }

    [[nodiscard]] constexpr auto
    charToRow (char chr) noexcept
        -> int
    {
        return 8 - (toLower (chr) - '0');
    }

    [[nodiscard]] constexpr auto
    charToCol (char chr) noexcept
        -> int
    {
        return toLower (chr) - 'a';
    }

    [[nodiscard]] constexpr auto
    rowToChar (int8_t row)
        -> char
    {
        ASSERT( isValidRow (row) );
        return narrow<char> (8 - row + '0');
    }

    [[nodiscard]] constexpr auto
    colToChar (int8_t col)
        -> char
    {
        ASSERT( isValidColumn (col) );
        return narrow<char> (col + 'a');
    }

    [[nodiscard]] auto
    asString (Coord coord)
        -> string;

    [[nodiscard]] constexpr auto
    coordParseOptional (string_view str) noexcept
        -> optional<Coord>
    {
        if (str.size() != 2)
            return nullopt;

        int col = charToCol (str[0]);
        int row = charToRow (str[1]);

        if (!isValidRow (row) || !isValidColumn (col))
            return nullopt;

        return makeCoord (row, col);
    }

    // A coordinate that does not parse is a precondition failure. Text from
    // outside the program goes through coordParseOptional().
    [[nodiscard]] constexpr auto
    coordParse (string_view str)
        -> Coord
    {
        auto result = coordParseOptional (str);
        EXPECTS( result.has_value() );
        return *result;
    }

    auto operator<< (std::ostream& ostream, Coord coord) -> std::ostream&;

    class CoordIterator
    {
    public:
        using difference_type = int;
        using value_type = Coord;
        using reference = Coord;
        using iterator_category = std::forward_iterator_tag;

        constexpr
        CoordIterator() noexcept
            : my_coord { First_Coord }
        {}

        explicit constexpr
        CoordIterator (Coord coord) noexcept
            : my_coord (coord)
        {}

        [[nodiscard]] constexpr auto
        begin() const noexcept
            -> CoordIterator
        {
            return *this;
        }

        [[nodiscard]] constexpr auto
        end() const noexcept  // NOLINT(readability-convert-member-functions-to-static)
            -> CoordIterator
        {
            return CoordIterator { End_Coord };
        }

        [[nodiscard]] constexpr auto
        operator*() const noexcept
            -> Coord
        {
            return my_coord;
        }

        constexpr auto
        operator++() noexcept
            -> CoordIterator&
        {
            ++my_coord;
            return *this;
        }

        constexpr auto
        operator++ (int) noexcept
            -> CoordIterator
        {
            auto previous = *this;
            ++my_coord;
            return previous;
        }

        [[nodiscard]] constexpr auto
        operator== (const CoordIterator& other) const noexcept
            -> bool
        {
            return other.my_coord == my_coord;
        }

        [[nodiscard]] constexpr auto
        operator!= (const CoordIterator& other) const noexcept
            -> bool
        {
            return !(*this == other);
        }

    private:
        Coord my_coord {};
    };
    static_assert (std::forward_iterator<CoordIterator>);

    // The color of the square.
    [[nodiscard]] constexpr auto
    coordColor (Coord coord) noexcept
        -> Color
    {
        int parity = (coord.row() % 2 + coord.column() % 2) % 2;
        return colorFromColorIndex (narrow_debug<int8_t> (parity));
    }

    // The row direction a pawn moves in: white moves up (-), black
    // moves down (+).
    template <class IntegerType = int8_t>
    [[nodiscard]] constexpr auto
    pawnDirection (Color color) noexcept
        -> IntegerType
    {
        static_assert (std::is_integral_v<IntegerType>);
        ASSERT( color == Color::Black || color == Color::White );
        auto color_as_int = to_int (color);
        return narrow_debug<IntegerType> (-1 + 2 * (color_as_int - 1));
    }
}
