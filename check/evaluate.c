#include <stdlib.h>
#include <stdio.h>

#include "evaluate.h"
#include "board.h"
#include "piece.h"
#include "check.h"
#include "search.h"
#include "material.h"

#include "debug.h"

DEFINE_DEBUG_CHANNEL (evaluate, 0);

/*
 * We use material.c for calculating the score after each move
 * is made. That should be less expensive overall, but it may
 * not be a good idea, because the do_move/undo_move path is
 * very critical. A transposition table would probably make
 * it even more so..
 */
#define USE_MATERIAL_STRUCT_INSTEAD 1

#ifndef USE_MATERIAL_STRUCT_INSTEAD
int const piece_weights[] =
{
	/* PIECE_NONE   */     0,
	/* PIECE_KING   */     1500,
	/* PIECE_QUEEN  */     1000,
	/* PIECE_ROOK   */     500,
	/* PIECE_BISHOP */     320,
	/* PIECE_KNIGHT */     320,
	/* PIECE_PAWN   */     100,
};
#endif

#define CASTLE_POSITIVE_WEIGHT        60 /* trying to catch a bug */
#define CASTLE_NEGATIVE_WEIGHT        60 /* trying to catch a bug */

/* stupid piece player 
 * this could be improved by just keeping the piece count in the board */
int evaluate (struct board *board, color_t who, int examine_checkmate, 
              move_t *move)
{
	int        score;
#ifndef USE_MATERIAL_STRUCT_INSTEAD
	signed int piece_type;
	int        row, col;
	piece_t    piece;
#endif

	/* this makes white -> -1, black -> 1 */
	const int  direction  = PAWN_DIRECTION (who);
	const int  opponent   = -direction;

	DBG (evaluate, "direction = %d\n", direction);
	score = 0;

#if 0
	if (who == COLOR_BLACK && !strcasecmp (move_str (*move), "O-O-O"))
		printf ("black would want to castle here\n");
#endif

	if (board->castled[who] == CASTLE_CASTLED)
		score += CASTLE_POSITIVE_WEIGHT;
	else 
	{
		if (!able_to_castle (board, who, CASTLE_KINGSIDE))
			score -= CASTLE_NEGATIVE_WEIGHT;
		if (!able_to_castle (board, who, CASTLE_QUEENSIDE))
			score -= CASTLE_NEGATIVE_WEIGHT;
	}

	if (board->castled[opponent] == CASTLE_CASTLED)
		score -= CASTLE_NEGATIVE_WEIGHT;
	else
	{
		if (!able_to_castle (board, opponent, CASTLE_KINGSIDE))
			score += CASTLE_POSITIVE_WEIGHT;
		if (!able_to_castle (board, opponent, CASTLE_QUEENSIDE))
			score += CASTLE_POSITIVE_WEIGHT;
	}

	if (is_king_threatened (board, who, ROW    (board->king_pos[who]),
	                                    COLUMN (board->king_pos[who])))
	{
		if (is_checkmated (board, who))
			return -INFINITY;
	}
#if 0
	if (examine_checkmate)
	{
		/* this incurs a sever speed penalty -- dont do it often */
		if (is_checkmated (board, who))
			return -INFINITY;
	}
#endif

	/* obsoleted by struct material */
#ifndef USE_MATERIAL_STRUCT_INSTEAD
	for_each_position (row, col)
	{
#if 1
		piece = PIECE_AT (board, row, col);

		/* -1 -> white, 1 -> black */
		piece_type = PAWN_DIRECTION (PIECE_COLOR (piece));

		/* -1 * -1 (white) == +1, 1 * 1 (black) == +1 */
		score += piece_weights[PIECE_TYPE (piece)] * piece_type 
		                                           * direction;
#else
		piece = PIECE_AT (board, row, col);

		if (PIECE_COLOR (piece) == who)
			score += piece_weights[PIECE_TYPE (piece)];
		else
			score -= piece_weights[PIECE_TYPE (piece)];
#endif
	}
#endif /* USE_MATERIAL */

	score += material_score (board->material, who);

#if 0
	if (move)
		DBG (evaluate, "move = %s\n", move_str (*move));
	DBG (evaluate, "score = %d\n", score);
	DBG (evaluate, "player = %s\n", who == COLOR_WHITE ?"white":"black");
	board_print (board);
#endif

	return score;
}

/* vi: set ts=4 sw=4: */
