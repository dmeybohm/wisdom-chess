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
        auto heuristic_is_castled (const Board& board, Color who) -> bool
        {
            auto king_pos = board.get_king_position (who);
            auto king_column = Column<int> (king_pos);
            auto king_row = Row<int> (king_pos);

            auto rook_piece = make_piece (who, Piece::Rook);

            if (king_row != castling_row_for_color (who))
                return false;

            // check kingside castle:
            if (king_column == Kingside_Castled_King_Column)
            {
                if (board.piece_at (king_row, Kingside_Castled_Rook_Column) == rook_piece)
                    return true;
            }
            else if (king_column == Queenside_Castled_King_Column)
            {
                if (board.piece_at (king_row, Queenside_Castled_Rook_Column) == rook_piece)
                    return true;
            }

            return false;
        }

        auto unable_to_castle_penalty (const Board& board, Color who) -> int
        {
            auto castle_state = board.get_castle_state (who);
            int result = 0;
            if (castle_state != Castle_None)
            {
                if (castle_state & Castle_Kingside)
                    result += Castle_Penalty;
                if (castle_state & Castle_Queenside)
                    result += Castle_Penalty;
                if (heuristic_is_castled (board, who))
                    result -= 2 * Castle_Penalty;
            }
            return result;
        }
    }

    auto evaluate (Board& board, Color who, int moves_away, MoveGenerator& generator) -> int
    {
        int score = 0;
        Color opponent = color_invert (who);

        score += board.get_material ().overall_score (who);
        score += board.get_position ().overall_score (who);

        if (is_checkmated (board, who, generator))
        {
            score = -1 * checkmate_score_in_moves (moves_away);
        }
        else if (is_checkmated (board, opponent, generator))
        {
            score = checkmate_score_in_moves (moves_away);
        }

        score -= unable_to_castle_penalty (board, who);
        score += unable_to_castle_penalty (board, opponent);

        return score;
    }

    auto evaluate_without_legal_moves (Board& board, Color who, int moves_away) -> int
    {
        auto king_coord = board.get_king_position (who);
        return  is_king_threatened (board, who, king_coord)
                       ? -1 * checkmate_score_in_moves (moves_away)
                       : 0;
    }
}
