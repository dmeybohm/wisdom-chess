#ifndef WISDOM_GLOBAL_H
#define WISDOM_GLOBAL_H

#include <cstdint>
#include <cassert>
#include <exception>

namespace wisdom
{
    enum
    {
        Num_Players = 2,

        Num_Rows = 8,
        Num_Columns = 8,

        First_Row = 0,
        First_Column = 0,

        Last_Row = 7,
        Last_Column = 7,

        King_Column = 4,
        King_Rook_Column = 7,

        Queen_Rook_Column = 0,
        King_Castled_Rook_Column = 5,
        Queen_Castled_Rook_Column = 3,
    };
}

#endif //WISDOM_GLOBAL_H
