#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "move.h"
#include "board.h"
#include "generate.h"
#include "search.h"
#include "piece.h"
#include "check.h"
#include "game.h"
#include "str.h"

static void print_available_moves (struct game *game)
{
	move_list_t *moves = NULL;
	move_t      *mptr;

	moves = generate_legal_moves (game->board, game->turn, game->history);

	printf ("\nAvailable moves: ");

	for_each_move (mptr, moves)
		printf ("[%s] ", move_str (*mptr));

	move_list_destroy (moves);

	printf("\n\n");
}

static int read_move (struct game *game, int *good, int *skip, move_t *move)
{
	char         buf[128];
#if 0
	piece_t src_piece, dst_piece;
#endif
	move_list_t *moves = NULL;
	move_t      *mptr;

	printf ("move? ");
	fflush (stdout);

	if (fgets (buf, sizeof (buf)-1, stdin) == NULL)
	{
		*good = 0;
		return 0;
	}

	chomp (buf);

	if (!strncmp (buf, "moves", strlen ("moves")))
	{
		print_available_moves (game);
		*skip = 1;
		return 1;
	}
	else if (!strncmp (buf, "save", strlen ("save")))
	{
		game_save (game);
		*skip = 1;
		return 1;
	}

#if 0
	printf ("src_row = %d src_col = %d\n", src_row, dst_col);
	printf ("dst_row = %d dst_col = %d\n", dst_row, dst_col);
	*good = 1;

	if (INVALID (src_row) || INVALID (src_col))
		*good = 0;

	if (INVALID (dst_row) || INVALID (dst_col))
		*good = 0;

	src_piece = PIECE_AT (board, src_row, src_col);
	dst_piece = PIECE_AT (board, dst_row, dst_col);

	if (PIECE_COLOR (src_piece) != side)
	{
		*good = 0;
	}
	else if (PIECE_TYPE (dst_piece) != PIECE_NONE && 
	         PIECE_COLOR (src_piece) == PIECE_COLOR (dst_piece))
	{
		*good = 0;
	}

#endif
	if (!move_parse (buf, game->turn, move))
		move_nullify (move);

	*good = 0;

	/* check the generated move list for this move to see if its valid */
	moves = generate_legal_moves (game->board, game->turn, game->history);

	for_each_move (mptr, moves)
		if (move_equal (mptr, move))
			*good = 1;

	move_list_destroy (moves);

	return 1;
}

int main (int argc, char **argv)
{
	struct game  *game;
	int           ok = 1, good;
	int           skip;
	move_t        move;
	color_t       comp_player = COLOR_BLACK;

	move_nullify (&move);

	game = game_new (COLOR_WHITE, comp_player);

	while (ok)
	{
		skip = 0;

		if (is_checkmated (game->board, game->turn))
		{
			printf ("%s wins the game\n", game->turn == COLOR_WHITE ? "Black" : 
			        "White");
			return 0;
		}

		if (game->turn != game->player)
		{
			board_print (game->board);
			ok = read_move (game, &good, &skip, &move);
		}
		else
		{
			board_print (game->board);
			move = find_best_move (game->board, game->player, game->history);

			printf ("move selected: [%s]\n", move_str (move));
			good = 1;
			ok = 1;
		}

		if (skip)
			continue;

		if (good)
			game_move (game, &move);
		else if (ok)
			printf("\nInvalid move\n\n");
	}

	return 0;
}

/* vi: set ts=4 sw=4: */
