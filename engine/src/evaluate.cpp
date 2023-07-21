#include "evaluate.hpp"
#include "board.hpp"
#include "check.hpp"
#include "position.hpp"
#include "search.hpp"

namespace wisdom
{
    static constexpr int Castle_Penalty = 50;

    namespace
    {
        auto heuristicIsCastled (const Board& board, Color who) -> bool
        {
            auto king_pos = board.getKingPosition (who);
            auto king_column = Column<int> (king_pos);
            auto king_row = Row<int> (king_pos);

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

        auto unableToCastlePenalty (const Board& board, Color who) -> int
        {
            auto castle_state = board.getCastlingEligibility (who);
            int result = 0;
            if (castle_state != CastlingEligible::EitherSideEligible)
            {
                if (castle_state & CastlingEligible::KingsideIneligible)
                    result += Castle_Penalty;
                if (castle_state & CastlingEligible::QueensideIneligible)
                    result += Castle_Penalty;
                if (heuristicIsCastled (board, who))
                    result -= 2 * Castle_Penalty;
            }
            return result;
        }
    }

    auto evaluate (const Board& board, Color who, int moves_away, MoveGenerator& generator) -> int
    {
        int score = 0;
        Color opponent = colorInvert (who);

        score += board.getMaterial().overallScore (who);
        score += board.getPosition ().overall_score (who);

        if (isCheckmated (board, who, generator))
        {
            score = -1 * checkmateScoreInMoves (moves_away);
        }
        else if (isCheckmated (board, opponent, generator))
        {
            score = checkmateScoreInMoves (moves_away);
        }

        score -= unableToCastlePenalty (board, who);
        score += unableToCastlePenalty (board, opponent);

        return score;
    }

    auto evaluateWithoutLegalMoves (const Board& board, Color who, int moves_away) -> int
    {
        auto king_coord = board.getKingPosition (who);
        return isKingThreatened (board, who, king_coord)
                       ? -1 * checkmateScoreInMoves (moves_away)
                       : 0;
    }
}
