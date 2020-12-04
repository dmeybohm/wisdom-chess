#ifndef WIZDUMB_GLOBAL_H
#define WIZDUMB_GLOBAL_H

#include <cstdint>
#include <cassert>
#include <exception>

enum
{
    NR_PLAYERS = 2,

    NR_ROWS = 8,
    NR_COLUMNS = 8,

    FIRST_ROW = 0,
    FIRST_COLUMN = 0,

    LAST_ROW = 7,
    LAST_COLUMN = 7,

    KING_COLUMN = 4,
    KING_ROOK_COLUMN = 7,

    QUEEN_ROOK_COLUMN = 0,
    KING_CASTLED_ROOK_COLUMN = 5,
    QUEEN_CASTLED_ROOK_COLUMN = 3,
};

#ifdef _MSC_VER
 #define strncasecmp _strnicmp
 #define strcasecmp _stricmp
#endif

#endif //WIZDUMB_GLOBAL_H
