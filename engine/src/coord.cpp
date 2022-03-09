#include "coord.hpp"
#include "move.hpp"

namespace wisdom
{
    auto coord_parse (const string& str) -> Coord
    {
        if (str.size () != 2)
            throw CoordParseError ("Invalid algebraic coordinate!");

        int col = char_to_col (str.at (0));
        int row = char_to_row (str.at (1));

        if (!is_valid_row (row) || !is_valid_column (col))
            throw CoordParseError ("Invalid algebraic coordinate!");

        return make_coord (row, col);
    }

    auto to_string (Coord coord) -> string
    {
        string result = ""; // NOLINT(readability-redundant-string-init)
        result += col_to_char (Column (coord));
        result += row_to_char (Row (coord));
        return result;
    }
}