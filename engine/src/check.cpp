#include "check.hpp"
#include "board.hpp"
#include "coord.hpp"
#include "generate.hpp"
#include "history.hpp"
#include "move.hpp"
#include "threats.hpp"

namespace wisdom
{
    bool isCheckmated (const Board& board, Color who)
    {
        auto coord = board.getKingPosition (who);

        if (!isKingThreatened (board, who, coord))
            return false;

        MoveList legal_moves = MoveGenerator::generateLegalMoves (board, who);

        return legal_moves.isEmpty();
    }

    auto isLegalPositionAfterMove (const Board& board, Color who, Move mv) -> bool
    {
        auto king_coord = board.getKingPosition (who);

        if (isKingThreatened (board, who, king_coord))
            return false;

        if (mv.isCastling())
        {
            Coord castled_pos = mv.getDst();
            auto castled_row = castled_pos.row();
            auto castled_col = castled_pos.column();

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
        auto legal_moves = MoveGenerator::generateLegalMoves (board, who);

        return legal_moves.isEmpty() && !isKingThreatened (board, who, coord);
    }
}
