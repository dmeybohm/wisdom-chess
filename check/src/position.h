
#ifndef WIZDUMB_POSITION_H
#define WIZDUMB_POSITION_H

#include "global.h"
#include "coord.h"
#include "move.h"
#include "piece.h"

struct position
{
    int score[NR_PLAYERS];
};

/////////////////////////////////////////////////////////

void position_init      (struct position *position);
void position_add       (struct position *position, Color who,
                         coord_t coord, piece_t piece);
void position_remove    (struct position *position, Color who,
                         coord_t coord, piece_t piece);

void position_do_move   (struct position *position, Color who,
                         piece_t piece, move_t move, undo_move_t undo_state);
void position_undo_move (struct position *position, Color who,
                         piece_t piece, move_t move, undo_move_t undo_state);

int  position_score     (const struct position *position, Color who);

/////////////////////////////////////////////////////////

#endif //WIZDUMB_POSITION_H
