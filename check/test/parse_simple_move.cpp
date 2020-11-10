#include "parse_simple_move.hpp"

move_t parse_move (const char *str, enum color color)
{
    move_t result = move_parse (str, COLOR_WHITE);
    if (color == COLOR_NONE &&
        result.move_category != MOVE_CATEGORY_NORMAL_CAPTURE &&
        result.move_category != MOVE_CATEGORY_NON_CAPTURE)
    {
        throw parse_move_exception("Invalid type of move in parse_simple_move");
    }
    if (is_null_move(result))
    {
        throw parse_move_exception ("Error parsing move");
    }
    return result;
}