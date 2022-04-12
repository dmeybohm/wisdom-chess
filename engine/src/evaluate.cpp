#include "evaluate.hpp"
#include "board.hpp"
#include "check.hpp"
#include "position.hpp"
#include "search.hpp"

namespace wisdom
{
    static const int Castle_Positive_Weight = 60;
    static const int Castle_Negative_Weight = 60;

    int evaluate (Board& board, Color who, int moves_away)
    {
        int score = 0;
        Color opponent = color_invert (who);

        score += board.get_material ().score (who);
        score += board.get_position ().score (who);

        if (is_checkmated (board, who))
        {
            score = -1 * checkmate_score_in_moves (moves_away);
        }
        else if (is_checkmated (board, opponent))
        {
            score = checkmate_score_in_moves (moves_away);
        }

        return score;
    }

    int evaluate_and_check_draw (Board& board, Color who, int moves_away, Move move,
                                 const History& history)
    {
        if (is_drawing_move (board, who, move, history))
        {
            return 0;
        }
        else
        {
            return evaluate (board, who, moves_away);
        }
    }
}
