
#ifndef WIZDUMB_POSITION_H
#define WIZDUMB_POSITION_H

#include "global.h"
#include "coord.h"
#include "piece.h"

struct position
{
    int score[NR_PLAYERS];
};

/////////////////////////////////////////////////////////

void position_init (struct position *position);
void position_add (struct position *position, coord_t coord, color_t who, piece_t piece);
void position_remove (struct position *position, coord_t coord, color_t who, piece_t piece);

/////////////////////////////////////////////////////////

#endif //WIZDUMB_POSITION_H
