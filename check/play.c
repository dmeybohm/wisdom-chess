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

static void chomp (char *str)
{
	if (str)
		if (str[strlen(str)-1] == '\n')
			str[strlen(str)-1] = 0;
}

static void save_game (struct board *board, color_t side, 
                       move_tree_t *history)
{
	FILE *out;
	static char buf[128];

	printf ("save to what file? ");
	fflush (stdout);

	if (fgets (buf, sizeof (buf)-1, stdin) == NULL)
		return;

	chomp (buf);

	if (!(out = fopen (buf, "w+")))
	{
		printf ("error opening file: %s", strerror (errno));
		return;
	}

	while (!is_null_move (history->move))
	{
		fprintf (out, "%s\n", move_str (history->move));
		history = history->parent;
	}

	fclose (out);
}

#if 0
static void load_game (struct board **r_board, color_t *r_side, 
                       move_tree_t *history)
{
}
#endif

static void print_available_moves (struct board *board, color_t side, 
                                   move_tree_t *history)
{
	move_list_t *moves = NULL;
	move_t      *mptr;

	moves = generate_legal_moves (board, side, history);

	printf ("\nAvailable moves: ");
	for_each_move (mptr, moves)
		printf ("[%s] ", move_str (*mptr));

	printf("\n\n");
}

static int read_move (struct board *board, color_t side, 
                      move_tree_t *history, int *good, int *skip, move_t *move)
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
		print_available_moves (board, side, history);
		*skip = 1;
		return 1;
	}
	else if (!strncmp (buf, "save", strlen ("save")))
	{
		save_game (board, side, history);
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
	if (!move_parse (buf, side, move))
		move_nullify (move);

	*good = 0;

	/* check the generated move list for this move to see if its valid */
	moves = generate_legal_moves (board, side, history);

	for_each_move (mptr, moves)
		if (move_equal (mptr, move))
			*good = 1;

	move_list_destroy (moves);

	return 1;
}

int main (int argc, char **argv)
{
	struct board *board;
	int           ok, good;
	int           turn, skip;
	color_t       side = COLOR_WHITE;
	move_tree_t  *history;
	move_t        move;

	move_nullify (&move);

	history = move_tree_new (NULL, move);

	board   = board_new ();

	turn = 0;
	do
	{
		skip = 0;
		if (is_checkmated (board, side))
		{
			printf ("%s wins the game\n", side == COLOR_WHITE ? "Black" : 
			        "White");
			return 0;
		}

		if (!turn)
		{
			board_print (board);
			ok = read_move (board, side, history, &good, &skip, &move);
		}
		else
		{
			board_print (board);
			move = find_best_move (board, side, history);

			printf ("move selected: [%s]\n", move_str (move));
			good = 1;
			ok = 1;
		}

		if (skip)
			continue;

		if (good)
		{
			do_move (board, side, &move);

			side = !side;
			turn = !turn;

			/* TODO: make do_move call move_tree_new() */
			history = move_tree_new (history, move);
		}
		else if (ok)
			printf("\nInvalid move\n\n");
	}
	while (ok);

	return 0;
}

/* vi: set ts=4 sw=4: */
