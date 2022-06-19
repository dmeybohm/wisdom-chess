#ifndef WISDOM_CHESS_COORD_ITERATOR_HPP
#define WISDOM_CHESS_COORD_ITERATOR_HPP

#include "global.hpp"
#include "coord.hpp"

namespace wisdom
{
    class CoordIterator final
    {
    private:
        Coord my_coord;

    public:
        CoordIterator () = default;

        explicit CoordIterator (Coord coord)
            : my_coord (coord)
        {}

        CoordIterator (int row_, int col_)
            : my_coord (make_coord (row_, col_))
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

        auto operator == (const CoordIterator &other) const -> bool
        {
            return other.my_coord == my_coord;
        }

        auto operator != (const CoordIterator &other) const -> bool
        {
            return !(*this == other);
        }
    };
}

#endif //WISDOM_CHESS_COORD_ITERATOR_HPP
