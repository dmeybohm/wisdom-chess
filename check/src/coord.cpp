#include "coord.h"
#include "move.h"

coord_t coord_parse (std::string_view str)
{
    if (str.size() != 2)
        throw coord_parse_exception("Invalid algebraic coordinate!");

    int8_t col = char_to_col(str[0]);
    int8_t row = char_to_row(str[1]);

    if (!VALID(row) || !VALID(col))
        throw coord_parse_exception("Invalid algebraic coordinate!");

    return make_coord (row, col);
}

std::string to_string (coord_t coord)
{
    std::string result = "";
    result += col_to_char(COLUMN(coord));
    result += row_to_char(ROW(coord));
    return result;
}