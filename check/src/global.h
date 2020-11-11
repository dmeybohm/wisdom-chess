#ifndef WIZDUMB_GLOBAL_H
#define WIZDUMB_GLOBAL_H

#include <stdint.h>
#include <assert.h>

constexpr unsigned int NR_PLAYERS = 2;

constexpr unsigned int NR_ROWS = 8;
constexpr unsigned int NR_COLUMNS = 8;

constexpr unsigned int LAST_ROW = NR_ROWS - 1;
constexpr unsigned int LAST_COLUMN = NR_COLUMNS - 1;

constexpr unsigned int KING_COLUMN = 4;
constexpr unsigned int KING_ROOK_COLUMN = 7;

constexpr unsigned int QUEEN_ROOK_COLUMN = 0;
constexpr unsigned int KING_CASTLED_ROOK_COLUMN = 5;
constexpr unsigned int QUEEN_CASTLED_ROOK_COLUMN = 3;


#endif //WIZDUMB_GLOBAL_H
