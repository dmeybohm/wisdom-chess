#pragma once

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
        [[nodiscard]] auto 
        overallScore (Color who) const 
            -> int;

        // The score for the individual player.
        [[nodiscard]] auto 
        individualScore (Color who) const 
            -> int;

        // Apply the move to the position.
        void applyMove (Color who, ColoredPiece src_piece, Move move, ColoredPiece dst_piece);

        friend auto 
        operator<< (std::ostream& ostream, Position& position) 
            -> std::ostream&;

    private:
        void add (Color who, Coord coord, ColoredPiece piece);
        void remove (Color who, Coord coord, ColoredPiece piece);

    private:
        int my_score[Num_Players]{};
    };
}

