#ifndef WIZDUMB_COORD_ITERATOR_HPP
#define WIZDUMB_COORD_ITERATOR_HPP

#include "coord.h"

class coord_iterator final
{
private:
    int8_t row = 0;
    int8_t col = 0;

public:
    coord_iterator () = default;

    coord_iterator (int8_t row_, int8_t col_)
        : row(row_), col(col_)
    {}

    [[nodiscard]] coord_iterator begin () // NOLINT(readability-convert-member-functions-to-static)
    {
        return coord_iterator (0, 0);
    }

    [[nodiscard]] coord_iterator end () // NOLINT(readability-convert-member-functions-to-static)
    {
        return coord_iterator (8, 0);
    }

    coord_t operator* () const
    {
        return make_coord (row, col);
    }

    coord_iterator& operator++ ()
    {
        col++;
        if (col == NR_COLUMNS)
        {
            row++;
            col = 0;
        }
        return *this;
    }

    bool operator == (const coord_iterator &other) const
    {
        return row == other.row && col == other.col;
    }

    bool operator != (const coord_iterator &other) const
    {
        return !(*this == other);
    }
};

extern coord_iterator all_coords_iterator;

#endif //WIZDUMB_COORD_ITERATOR_HPP
