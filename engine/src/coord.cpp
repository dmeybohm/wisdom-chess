#include "board.hpp"
#include "board_builder.hpp"
#include "coord.hpp"
#include "move.hpp"

namespace wisdom
{
    auto coordParse (const string& str) -> Coord
    {
        if (str.size() != 2)
            throw CoordParseError ("Invalid coordinate!");

        int col = charToCol (str.at (0));
        int row = charToRow (str.at (1));

        if (!isValidRow (row) || !isValidColumn (col))
            throw CoordParseError ("Invalid coordinate!");

        return makeCoord (row, col);
    }

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