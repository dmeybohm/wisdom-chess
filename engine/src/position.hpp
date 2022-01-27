
#ifndef WISDOM_POSITION_H
#define WISDOM_POSITION_H

#include "global.hpp"
#include "coord.hpp"
#include "move.hpp"
#include "piece.hpp"

namespace wisdom
{
    class Position
    {
    private:
        int my_score[Num_Players]{};

    public:
        Position() = default;

        [[nodiscard]] int score (Color who) const;

        [[nodiscard]] int raw_score (Color who) const;

        void add (Color who, Coord coord, ColoredPiece piece);
        void remove (Color who, Coord coord, ColoredPiece piece);
        void apply_move (Color who, ColoredPiece piece, Move move, const UndoMove& undo_state);
        void unapply_move (Color who, ColoredPiece piece, Move move, const UndoMove& undo_state);
    };
}

#endif //WISDOM_POSITION_H
