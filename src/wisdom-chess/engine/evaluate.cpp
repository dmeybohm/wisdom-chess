#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/position.hpp"
#include "wisdom-chess/engine/threats.hpp"

namespace wisdom
{
    namespace
    {
        constexpr int Castle_Penalty = 50;

        [[nodiscard]] auto
        heuristicIsCastled (const Board& board, Color who) noexcept
            -> bool
        {
            auto king_pos = board.getKingPosition (who);
            auto king_column = king_pos.column<int>();
            auto king_row = king_pos.row<int>();

            auto rook_piece = ColoredPiece::make (who, Piece::Rook);

            if (king_row != castlingRowForColor (who))
                return false;

            // check kingside castle:
            if (king_column == Kingside_Castled_King_Column)
            {
                if (board.pieceAt (king_row, Kingside_Castled_Rook_Column) == rook_piece)
                    return true;
            }
            else if (king_column == Queenside_Castled_King_Column)
            {
                if (board.pieceAt (king_row, Queenside_Castled_Rook_Column) == rook_piece)
                    return true;
            }

            return false;
        }

        [[nodiscard]] auto
        unableToCastlePenalty (const Board& board, Color who) noexcept
            -> int
        {
            auto castle_state = board.getCastlingEligibility (who);
            int result = 0;
            if (castle_state != CastlingEligibility::Both_Sides)
            {
                if (!castle_state.canCastleKingside())
                    result += Castle_Penalty;
                if (!castle_state.canCastleQueenside())
                    result += Castle_Penalty;
                if (heuristicIsCastled (board, who))
                    result -= 2 * Castle_Penalty;
            }
            return result;
        }
    }

    auto
    evaluate (const Board& board, Color who, int moves_away) noexcept
        -> int
    {
        if (isCheckmated (board))
        {
            int sign = who == board.getCurrentTurn() ? -1 : 1;
            return sign * checkmateScoreInMoves (moves_away);
        }

        return evaluateWithoutMateTest (board, who);
    }

    auto
    evaluateWithoutMateTest (const Board& board, Color who) noexcept
        -> int
    {
        int score = 0;
        Color opponent = colorInvert (who);

        score += board.getMaterial().overallScore (who);
        score += board.getPosition().overallScore (who);

        score -= unableToCastlePenalty (board, who);
        score += unableToCastlePenalty (board, opponent);

        // Anything larger would read as a checkmate score.
        ENSURES_NOEXCEPT( score < Max_Non_Checkmate_Score && score > -Max_Non_Checkmate_Score );
        return score;
    }

    auto
    evaluateWithoutLegalMoves (const Board& board, Color who, int moves_away) noexcept
        -> int
    {
        auto king_coord = board.getKingPosition (who);
        return isKingThreatened (board, who, king_coord)
            ? -1 * checkmateScoreInMoves (moves_away)
            : 0;
    }

}
