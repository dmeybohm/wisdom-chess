#include "check.hpp"
#include "board.hpp"
#include "coord.hpp"
#include "generate.hpp"
#include "history.hpp"
#include "move.hpp"
#include "threats.hpp"

namespace wisdom
{
    bool isCheckmated (const Board& board, Color who, MoveGenerator& generator)
    {
        auto coord = board.getKingPosition (who);

        if (!isKingThreatened (board, who, coord))
            return false;

        MoveList legal_moves = generator.generateLegalMoves (board, who);

        return legal_moves.empty ();
    }

    auto isLegalPositionAfterMove (const Board& board, Color who, Move mv) -> bool
    {
        auto king_coord = board.getKingPosition (who);

        if (isKingThreatened (board, who, king_coord))
            return false;

        auto king_row = Row (king_coord);
        auto king_col = Column (king_coord);

        if (mv.is_castling ())
        {
            Coord castled_pos = mv.get_dst ();
            auto castled_row = Row (castled_pos);
            auto castled_col = Column (castled_pos);

            assert (king_row == castled_row);
            assert (king_col == castled_col);

            int8_t direction = mv.is_castling_on_kingside () ? -1 : 1;

            int8_t plus_one_column = nextColumn (castled_col, direction);
            int8_t plus_two_column = nextColumn (plus_one_column, direction);

            if (isKingThreatened (board, who, castled_row, plus_one_column) || isKingThreatened (board, who, castled_row, plus_two_column))
            {
                return false;
            }

        }

        return true;
    }

    auto isStalemated (const Board& board, Color who, MoveGenerator& generator) -> bool
    {
        auto coord = board.getKingPosition (who);
        auto legal_moves = generator.generateLegalMoves (board, who);

        return legal_moves.empty () && !isKingThreatened (board, who, coord);
    }
}
