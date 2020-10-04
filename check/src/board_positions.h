#ifndef WIZDUMB_BOARD_POSITIONS_H
#define WIZDUMB_BOARD_POSITIONS_H

#include <stdlib.h>

#include "piece.h"

struct board_positions
{
    int               rank;
    enum color        piece_color;
    enum piece_type  *pieces;
};

struct board_positions *board_positions_dupe (struct board_positions *positions, size_t sz);
void board_position_free (struct board_positions *positions);

#endif //WIZDUMB_BOARD_POSITIONS_H
