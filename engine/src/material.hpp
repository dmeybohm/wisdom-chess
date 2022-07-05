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

        // Count of pieces on either side:
        array<array<int, Num_Pieces>, Num_Players> my_piece_count {};

    public:
        Material () = default;

        explicit Material (const Board& board);

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

        [[nodiscard]] static auto score_with_scale (int score) -> int
        {
            return score * Material_Score_Scale;
        }

        void add (ColoredPiece piece)
        {
            auto color_idx = color_index (piece_color (piece));
            auto type = piece_type (piece);

            my_score[color_idx] += weight (type);
            my_piece_count[color_idx][to_int (type)]++;

            assert (my_piece_count[color_idx][to_int (type)] > 0);
        }

        void remove (ColoredPiece piece)
        {
            auto color_idx = color_index (piece_color (piece));
            auto type = piece_type (piece);

            my_score[color_idx] -= weight (type);
            my_piece_count[color_idx][to_int (type)]--;

            assert (my_piece_count[color_idx][to_int (type)] >= 0);
        }

        [[nodiscard]] auto individual_score (Color who) const -> int
        {
            ColorIndex my_index = color_index (who);
            return score_with_scale (my_score[my_index]);
        }

        [[nodiscard]] auto overall_score (Color who) const -> int
        {
            ColorIndex my_index = color_index (who);
            ColorIndex opponent_index = color_index (color_invert (who));
            return score_with_scale (my_score[my_index] - my_score[opponent_index]);
        }

        [[nodiscard]] auto piece_count (Color who, Piece type) const -> int
        {
            auto color_idx = color_index (who);
            auto type_idx = to_int (type);

            return my_piece_count[color_idx][type_idx];
        }

        // Whether there is insufficient material remaining for a checkmate.
        [[nodiscard]] auto has_sufficient_material (const Board& board) const -> bool
        {
            if (piece_count (Color::White, Piece::Pawn) > 0 ||
                piece_count (Color::Black, Piece::Pawn) > 0 ||
                piece_count (Color::White, Piece::Rook) > 0 ||
                piece_count (Color::Black, Piece::Rook) > 0)
            {
                return true;
            }

            if (individual_score (Color::White) > score_with_scale (WeightKing + 2 * WeightBishop) ||
                individual_score (Color::Black) > score_with_scale (WeightKing + 2 * WeightBishop))
            {
                return true;
            }

            return !check_insufficient_material_scenarios (board);
        }

    private:
        // Check for more detailed scenarios of sufficient material. This assumes there are
        // only minor pieces and king left, with no pawns.
        [[nodiscard]] auto check_insufficient_material_scenarios (const Board& board) const -> bool;
    };
}

#endif // WISDOM_CHESS_MATERIAL_HPP
