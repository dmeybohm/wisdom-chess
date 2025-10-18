#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/position.hpp"
#include "wisdom-chess/engine/search.hpp"

namespace wisdom
{
    static constexpr int Castle_Penalty = 50;

    [[nodiscard]] static auto
    heuristicIsCastled (const Board& board, Color who)
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

    [[nodiscard]] static auto
    unableToCastlePenalty (const Board& board, Color who)
        -> int
    {
        auto castle_state = board.getCastlingEligibility (who);
        int result = 0;
        if (castle_state != CastlingEligibility::Either_Side)
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

    auto 
    evaluate (const Board& board, Color who, int moves_away) 
        -> int
    {
        int score = 0;
        Color opponent = colorInvert (who);

        if (isCheckmated (board, who))
        {
            return -1 * checkmateScoreInMoves (moves_away);
        }
        else if (isCheckmated (board, opponent))
        {
            return checkmateScoreInMoves (moves_away);
        }

        score += board.getMaterial().overallScore (who);
        score += board.getPosition().overallScore (who);

        score -= unableToCastlePenalty (board, who);
        score += unableToCastlePenalty (board, opponent);

        return score;
    }

    auto 
    evaluateWithoutLegalMoves (const Board& board, Color who, int moves_away) 
        -> int
    {
        auto king_coord = board.getKingPosition (who);
        return isKingThreatened (board, who, king_coord) 
            ? -1 * checkmateScoreInMoves (moves_away) 
            : 0;
    }

    bool isCheckmated (const Board& board, Color who)
    {
        auto coord = board.getKingPosition (who);

        if (!isKingThreatened (board, who, coord))
            return false;

        MoveList legal_moves = generateLegalMoves (board, who);

        return legal_moves.isEmpty();
    }

    auto
    isLegalPositionAfterMove (const Board& board, Color who, Move mv)
        -> bool
    {
        auto king_coord = board.getKingPosition (who);

        if (isKingThreatened (board, who, king_coord))
            return false;

        if (mv.isCastling())
        {
            Coord castled_pos = mv.getDst();
            auto castled_row = castled_pos.row();
            auto castled_col = castled_pos.column();

            assert (king_coord.row() == castled_row);
            assert (king_coord.column() == castled_col);

            int8_t direction = mv.isCastlingOnKingside() ? -1 : 1;

            int8_t plus_one_column = nextColumn (castled_col, direction);
            int8_t plus_two_column = nextColumn (plus_one_column, direction);

            if (isKingThreatened (board, who, castled_row, plus_one_column)
                || isKingThreatened (board, who, castled_row, plus_two_column))
            {
                return false;
            }
        }

        return true;
    }

    auto isStalemated (const Board& board, Color who) -> bool
    {
        auto coord = board.getKingPosition (who);
        auto legal_moves = generateLegalMoves (board, who);

        return legal_moves.isEmpty() && !isKingThreatened (board, who, coord);
    }
}
