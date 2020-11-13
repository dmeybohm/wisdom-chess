#include "coord.h"

coord_t coord_parse (const char *str)
{
    if (!str || strlen(str) == 2)
        throw coord_parse_exception("Invalid algebraic coordinate!");

    uint8_t row = char_to_col(str[0]);
    uint8_t col = char_to_row(str[1]);

    if (row >= NR_ROWS || col >= NR_COLUMNS)
        throw coord_parse_exception("Invalid algebraic coordinate!");

    return coord_create (row, col);
}