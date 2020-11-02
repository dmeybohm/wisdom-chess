
#ifndef WIZDUMB_BITBOARD_VECTOR_INFO_H
#define WIZDUMB_BITBOARD_VECTOR_INFO_H

#include <string>

extern "C"
{
#include "bitboard.h"
}

std::string per_player_bitboard_info (per_player_bitboard_t *bitboard,
                                      uint8_t bits_per_unit);

#endif //WIZDUMB_BITBOARD_VECTOR_INFO_H
