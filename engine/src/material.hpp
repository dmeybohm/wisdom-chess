#ifndef WISDOM_CHESS_MATERIAL_HPP
#define WISDOM_CHESS_MATERIAL_HPP

#include "global.hpp"
#include "piece.hpp"

namespace wisdom
{
    class Board;

    class Material
    {
    private:
        int my_score[Num_Players] {};

    public:
        Material () = default;

        explicit Material (const Board& board);

        enum MaterialWeight
        {
            WeightNone = 0,
            WeightKing = 1500,
            WeightQueen = 1000,
            WeightRook = 500,
            WeightBishop = 320,
            WeightKnight = 305,
            WeightPawn = 100,
        };

        static constexpr auto min_checkmate_weight () -> int
        {
            return WeightKing + WeightKnight * 2;
        }

        [[nodiscard]] static int weight (Piece piece) noexcept
        {
            switch (piece)
            {
                case Piece::None: return WeightNone;
                case Piece::King: return WeightKing;
                case Piece::Queen: return WeightQueen;
                case Piece::Rook: return WeightRook;
                case Piece::Bishop: return WeightBishop;
                case Piece::Knight: return WeightKnight;
                case Piece::Pawn: return WeightPawn;
            }
            std::terminate ();
        }

        void add (ColoredPiece piece)
        {
            my_score[color_index (piece_color (piece))] += weight (piece_type (piece));
        }

        void remove (ColoredPiece piece)
        {
            my_score[color_index (piece_color (piece))] -= weight (piece_type (piece));
        }

        [[nodiscard]] int individual_score (Color who) const
        {
            ColorIndex my_index = color_index (who);
            ColorIndex opponent_index = color_index (color_invert (who));
            return my_score[my_index];
        }

        [[nodiscard]] int overall_score (Color who) const
        {
            ColorIndex my_index = color_index (who);
            ColorIndex opponent_index = color_index (color_invert (who));
            return my_score[my_index] - my_score[opponent_index];
        }
    };
}

#endif // WISDOM_CHESS_MATERIAL_HPP
