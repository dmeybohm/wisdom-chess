#ifndef WISDOM_CHESS_COORD_ITERATOR_HPP
#define WISDOM_CHESS_COORD_ITERATOR_HPP

#include "global.hpp"
#include "coord.hpp"

namespace wisdom
{
    class CoordIterator final
    {
    private:
        int row = 0;
        int col = 0;

    public:
        CoordIterator () = default;

        CoordIterator (int row_, int col_)
                : row (row_), col (col_)
        {}

        [[nodiscard]] CoordIterator begin () // NOLINT(readability-convert-member-functions-to-static)
        {
            return { 0, 0 };
        }

        [[nodiscard]] CoordIterator end () // NOLINT(readability-convert-member-functions-to-static)
        {
            return { 8,  0 };
        }

        auto operator* () const -> Coord
        {
            return make_coord (row, col);
        }

        auto operator++ () -> CoordIterator&
        {
            col++;
            if (col == Num_Columns)
            {
                row++;
                col = 0;
            }
            return *this;
        }

        auto operator == (const CoordIterator &other) const -> bool
        {
            return row == other.row && col == other.col;
        }

        auto operator != (const CoordIterator &other) const -> bool
        {
            return !(*this == other);
        }
    };
}

#endif //WISDOM_CHESS_COORD_ITERATOR_HPP
