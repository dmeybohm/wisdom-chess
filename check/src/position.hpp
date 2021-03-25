
#ifndef WIZDUMB_POSITION_H
#define WIZDUMB_POSITION_H

#include "global.hpp"
#include "coord.hpp"
#include "move.hpp"
#include "piece.hpp"

namespace wisdom
{
    struct Position
    {
        int score[Num_Players];
    };

/////////////////////////////////////////////////////////

    void position_init (struct Position *position);

    void position_add (struct Position *position, Color who,
                       Coord coord, ColoredPiece piece);

    void position_remove (struct Position *position, Color who,
                          Coord coord, ColoredPiece piece);

    void position_do_move (struct Position *position, Color who,
                           ColoredPiece piece, Move move, UndoMove undo_state);

    void position_undo_move (struct Position *position, Color who,
                             ColoredPiece piece, Move move, UndoMove undo_state);

    int position_score (const struct Position *position, Color who);

/////////////////////////////////////////////////////////

}
#endif //WIZDUMB_POSITION_H
