#ifndef WIZDUMB_COORD_ITERATOR_HPP
#define WIZDUMB_COORD_ITERATOR_HPP

#include "coord.h"

struct coord_iterator
{
private:
    uint8_t row = 0;
    uint8_t col = 0;

public:
    constexpr coord_iterator () = default;

    constexpr coord_iterator (uint8_t _row, uint8_t _col)
        : row(_row), col(_col)
    {}

    [[nodiscard]] constexpr coord_iterator begin () // NOLINT(readability-convert-member-functions-to-static)
    {
        return coord_iterator (0, 0);
    }

    [[nodiscard]] constexpr coord_iterator end () // NOLINT(readability-convert-member-functions-to-static)
    {
        return coord_iterator (8, 0);
    }

    coord_t operator* () const
    {
        return coord_create (row, col);
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

#endif //WIZDUMB_COORD_ITERATOR_HPP
