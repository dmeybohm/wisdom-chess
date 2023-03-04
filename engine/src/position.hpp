#ifndef WISDOM_CHESS_POSITION_H
#define WISDOM_CHESS_POSITION_H

#include "global.hpp"
#include "coord.hpp"
#include "move.hpp"
#include "piece.hpp"

namespace wisdom
{
    class Board;

    class Position
    {
    public:
        Position() = default;

        explicit Position (const Board& board);

        // My score minus my oppponent's score
        [[nodiscard]] int overall_score (Color who) const;

        // The score for the individual player.
        [[nodiscard]] int individual_score (Color who) const;

        // Apply the move to the position.
        void apply_move (Color who, ColoredPiece src_piece, Move move, ColoredPiece dst_piece);

    private:
        int my_score[Num_Players]{};

        void add (Color who, Coord coord, ColoredPiece piece);
        void remove (Color who, Coord coord, ColoredPiece piece);
    };
}

#endif //WISDOM_CHESS_POSITION_H
