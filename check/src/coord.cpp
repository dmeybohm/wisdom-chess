#include "coord.h"

coord_t coord_parse (std::string_view str)
{
    if (str.size() != 2)
        throw coord_parse_exception("Invalid algebraic coordinate!");

    int8_t col = char_to_col(str[0]);
    int8_t row = char_to_row(str[1]);

    if (!VALID(row) || !VALID(col))
        throw coord_parse_exception("Invalid algebraic coordinate!");

    return coord_create (row, col);
}