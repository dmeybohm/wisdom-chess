#include "evaluate.h"
#include "board.h"
#include "check.h"
#include "search.h"
#include "position.h"

namespace wisdom
{
    static const int Castle_Positive_Weight = 60;
    static const int Castle_Negative_Weight = 60;

    int evaluate (Board &board, Color who, int moves_away)
    {
        int score = 0;
        Color opponent = color_invert (who);

        if (board_get_castle_state (board, who) == CASTLE_CASTLED)
        {
            score += Castle_Positive_Weight;
        }
        else
        {
            if (!able_to_castle (board, who, CASTLE_KINGSIDE))
                score -= Castle_Negative_Weight;
            if (!able_to_castle (board, who, CASTLE_QUEENSIDE))
                score -= Castle_Negative_Weight;
        }

        if (board_get_castle_state (board, opponent) == CASTLE_CASTLED)
        {
            score -= Castle_Negative_Weight;
        }
        else
        {
            if (!able_to_castle (board, opponent, CASTLE_KINGSIDE))
                score += Castle_Positive_Weight;
            if (!able_to_castle (board, opponent, CASTLE_QUEENSIDE))
                score += Castle_Positive_Weight;
        }

        score += board.material.score (who);
        score += position_score (&board.position, who);

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

    int evaluate_and_check_draw (Board &board, Color who, int moves_away,
                                 Move move, const History &history)
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
