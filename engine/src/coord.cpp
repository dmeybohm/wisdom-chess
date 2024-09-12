#include "board.hpp"
#include "board_builder.hpp"
#include "coord.hpp"
#include "move.hpp"

namespace wisdom
{
    auto asString (Coord coord) -> string
    {
        string result = ""; // NOLINT(readability-redundant-string-init)
        result += colToChar (coord.column());
        result += rowToChar (coord.row());
        return result;
    }

    auto operator<< (std::ostream& ostream, Coord coord) -> std::ostream&
    {
        ostream << asString (coord);
        return ostream;
    }
}