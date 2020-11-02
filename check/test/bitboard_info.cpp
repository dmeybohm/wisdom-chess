
#include "bitboard_info.hpp"

extern "C"
{
#include "bitboard.h"
}

#include <sstream>

std::string per_player_bitboard_info (per_player_bitboard_t *bitboard, uint8_t bits_per_unit)
{
    std::ostringstream ostr {};
    ostr << "{";
    for (int row = 0; row < 8; row++)
    {
        ostr << row << ": ";
        for (int col = 0; col < 8; col++)
        {
            ostr << "[ ";
            for (color_t color = COLOR_WHITE; color != COLOR_LAST; color++)
            {
                player_index_t player_index = color_to_player_index((enum color)color);

                int value = per_player_bitboard_get (bitboard, player_index, coord_create (row, col), bits_per_unit);

                ostr << value << " (" << (color == COLOR_WHITE ? "W" : "B") << ")" << ", ";
            }
            ostr << "], ";
        }
        ostr << "], ";
    }

    return ostr.str();
}
