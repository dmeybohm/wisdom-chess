#include "parse_simple_move.hpp"
#include <cctype>

move_t parse_move (const char *str, enum color color)
{
    if (tolower(str[0]) == 'o' && color == COLOR_NONE)
        throw parse_move_exception("Move requires color, but no color provided");
    move_t result = move_parse (str, color);
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