#include "coord.hpp"
#include "move.hpp"

namespace wisdom
{
    Coord coord_parse (const std::string &str)
    {
        if (str.size () != 2)
            throw CoordParseError ("Invalid algebraic coordinate!");

        int8_t col = char_to_col (str.at (0));
        int8_t row = char_to_row (str.at (1));

        if (!is_valid_row (row) || !is_valid_column (col))
            throw CoordParseError ("Invalid algebraic coordinate!");

        return make_coord (row, col);
    }

    std::string to_string (Coord coord)
    {
        std::string result = ""; // NOLINT(readability-redundant-string-init)
        result += col_to_char (COLUMN (coord));
        result += row_to_char (ROW (coord));
        return result;
    }
}