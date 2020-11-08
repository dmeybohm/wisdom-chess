#include "evaluate.h"
#include "board.h"
#include "check.h"
#include "search.h"
#include "debug.h"
#include "position.h"

DEFINE_DEBUG_CHANNEL (evaluate, 0);

#define CASTLE_POSITIVE_WEIGHT        60
#define CASTLE_NEGATIVE_WEIGHT        60

int evaluate (struct board *board, color_t who, int examine_checkmate)
{
	int score = 0;
	int opponent = color_invert (who);

	if (board_get_castle_state (board, who) == CASTLE_CASTLED)
	{
        score += CASTLE_POSITIVE_WEIGHT;
    }
	else 
	{
		if (!able_to_castle (board, who, CASTLE_KINGSIDE))
			score -= CASTLE_NEGATIVE_WEIGHT;
		if (!able_to_castle (board, who, CASTLE_QUEENSIDE))
			score -= CASTLE_NEGATIVE_WEIGHT;
	}

	if (board_get_castle_state (board, opponent) == CASTLE_CASTLED)
	{
        score -= CASTLE_NEGATIVE_WEIGHT;
    }
	else
	{
		if (!able_to_castle (board, opponent, CASTLE_KINGSIDE))
			score += CASTLE_POSITIVE_WEIGHT;
		if (!able_to_castle (board, opponent, CASTLE_QUEENSIDE))
			score += CASTLE_POSITIVE_WEIGHT;
	}

	score += material_score (&board->material, who);
    score += position_score (&board->position, who);

	coord_t king_pos = king_position (board, who);
	coord_t opponent_king_pos = king_position (board, opponent);
	if (is_king_threatened (board, who, ROW(king_pos), COLUMN(king_pos)))
	{
		if (is_checkmated (board, who))
			score = -INFINITE;
	}
	else if (is_king_threatened (board, opponent, ROW(opponent_king_pos), COLUMN(opponent_king_pos)))
    {
        if (is_checkmated (board, opponent))
            score = INFINITE;
    }

	return score;
}

int evaluate_and_check_draw (struct board *board, color_t who, int examine_checkmate,
                             move_t move, move_tree_t *history)
{
    if (is_drawing_move (history, move))
    {
        return 0;
    }
    else
    {
        return evaluate (board, who, examine_checkmate);
    }
}

// vi: set ts=4 sw=4:
