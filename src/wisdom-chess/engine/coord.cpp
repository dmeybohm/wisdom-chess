#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/board_builder.hpp"

namespace wisdom
{
    auto asString (Coord coord) -> string
    {
        string result = ""; // NOLINT(readability-redundant-string-init)
        result += colToChar (coord.column());
        result += rowToChar (coord.row());
        return result;
    }

    auto 
    operator<< (std::ostream& ostream, Coord coord) 
        -> std::ostream&
    {
        ostream << asString (coord);
        return ostream;
    }
}
