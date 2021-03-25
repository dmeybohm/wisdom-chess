#ifndef WISDOM_COORD_ITERATOR_HPP
#define WISDOM_COORD_ITERATOR_HPP

#include "coord.hpp"

namespace wisdom
{
    class CoordIterator final
    {
    private:
        int8_t row = 0;
        int8_t col = 0;

    public:
        CoordIterator () = default;

        CoordIterator (int8_t row_, int8_t col_)
                : row (row_), col (col_)
        {}

        [[nodiscard]] CoordIterator begin () // NOLINT(readability-convert-member-functions-to-static)
        {
            return CoordIterator (0, 0);
        }

        [[nodiscard]] CoordIterator end () // NOLINT(readability-convert-member-functions-to-static)
        {
            return CoordIterator (8, 0);
        }

        Coord operator* () const
        {
            return make_coord (row, col);
        }

        CoordIterator &operator++ ()
        {
            col++;
            if (col == Num_Columns)
            {
                row++;
                col = 0;
            }
            return *this;
        }

        bool operator== (const CoordIterator &other) const
        {
            return row == other.row && col == other.col;
        }

        bool operator!= (const CoordIterator &other) const
        {
            return !(*this == other);
        }
    };

    extern CoordIterator All_Coords_Iterator;
}
#endif //WISDOM_COORD_ITERATOR_HPP
