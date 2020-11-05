#include "evaluate.h"
#include "board.h"
#include "check.h"
#include "search.h"
#include "debug.h"
#include "position.h"

DEFINE_DEBUG_CHANNEL (evaluate, 0);

#define CHECK_PENALTY                 1

#define CASTLE_POSITIVE_WEIGHT        60
#define CASTLE_NEGATIVE_WEIGHT        60

int evaluate (struct board *board, color_t who, int examine_checkmate,
              move_t *move)
{
	int        score;

	// this makes white -> -1, black -> 1
	const int  direction  = PAWN_DIRECTION (who);
	const int  opponent   = color_invert (who);

	DBG (evaluate, "direction = %d\n", direction);
	score = 0;

	if (board_get_castle_state(board, who) == CASTLE_CASTLED)
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

	if (board_get_castle_state(board, opponent) == CASTLE_CASTLED)
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

	coord_t king_pos = king_position (board, who);
	if (is_king_threatened (board, who, ROW(king_pos), COLUMN(king_pos)))
	{
		if (is_checkmated (board, who))
			return -INFINITY;
		score -= CHECK_PENALTY;
	}

	score += material_score (&board->material, who);
    score += position_score (&board->position, who);

	return score;
}

int evaluate_and_check_draw (struct board *board, color_t who, int examine_checkmate,
              move_t *move, move_tree_t *history)
{
    if (is_drawing_move (history, move))
    {
        return 0;
    }
    else
    {
        return evaluate (board, who, examine_checkmate, move);
    }
}

// vi: set ts=4 sw=4:
