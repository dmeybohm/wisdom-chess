#include "parse_simple_move.hpp"

move_t parse_simple_move (const char *str)
{
    move_t result = move_parse (str, COLOR_WHITE);
    if (is_null_move(result))
    {
        throw parse_simple_move_exception("Error parsing move");
    }
    if (result.move_category != MOVE_CATEGORY_NORMAL_CAPTURE &&
        result.move_category != MOVE_CATEGORY_NON_CAPTURE)
    {
        throw parse_simple_move_exception("Invalid type of move in parse_simple_move");
    }
    return result;
}