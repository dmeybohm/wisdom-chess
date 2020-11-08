#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "piece.h"
#include "board.h"
#include "move.h"
#include "move_tree.h"
#include "str.h"
#include "game.h"
#include "board_check.h"

struct game *game_new (color_t turn, color_t computer_player)
{
	struct game *game;

	game = malloc (sizeof(*game));
	assert (game != NULL);

	game->board = board_new ();
	assert (game->board != NULL);

	game->history = NULL;

	// if 'turn' is something bogus, use white
	if (is_color_invalid (turn))
		turn = COLOR_WHITE;

	// if 'computer_player' is bogus, use black
	if (is_color_invalid (computer_player))
		computer_player = COLOR_BLACK;

	game->player = computer_player;
	game->turn   = turn;

	return game;
}

void game_free (struct game *game)
{
	if (!game)
		return;

	board_free (game->board);
	move_tree_free (game->history);

	free (game);
}

void game_move (struct game *game, move_t move)
{
	// add this move to the history
	game->history = move_tree_new (game->history, move);

	// do the move
    do_move (game->board, game->turn, move);

	// take our turn
	game->turn = color_invert (game->turn);
}

static int prompt (const char *prompt, char *out, size_t len)
{
	printf ("%s? ", prompt);
	fflush (stdout);

	if (fgets (out, len - 1, stdin) == NULL)
		return 0;

	chomp (out);
	return 1;
}

static int save_history_recursively (move_tree_t *history, FILE *f)
{
	int ret;

	if (!history || is_null_move (history->move))
		return 1;

	ret = save_history_recursively (history->parent, f);
	fprintf (f, "%s\n", move_str (history->move));

	return ret;
}

int game_save (struct game *game)
{
	FILE *out;
	char buf[128];

	if (!prompt ("save to what file", buf, sizeof (buf) - 1))
		return 0;

	//
	// Kludgily use append mode to preserve files
	//
	if (!(out = fopen (buf, "a+")))
	{
		printf ("error opening file: %s", strerror (errno));
		return 0;
	}

	if (!save_history_recursively (game->history, out))
	{
		fclose (out);
		return 0;
	}

	fclose (out);

	return 1; // need to check for failure here
}

struct game *game_load (color_t player)
{
	struct game *game;
	char file[128];
	char buf[128];
	FILE *f;
	move_t move;

	if (!prompt ("load what file", file, sizeof (file) - 1))
		return NULL;

	if (!(f = fopen (file, "r")))
	{
		printf ("Couldn't open %s: %s\n", file, strerror (errno));
		return NULL;
	}

	game = game_new (COLOR_NONE, player);

	while (fgets (buf, sizeof (buf) - 1, f) != NULL)
	{
		coord_t dst;
		piece_t piece;

		if (!strncasecmp (buf, "stop", strlen ("stop")))
			break;

		move = move_parse (buf, game->turn);
		if (is_null_move(move))
		{
			printf ("Error parsing game file %s: invalid move \"%s\"",
					file, buf);
			game_free (game);
			return NULL;
		}

		//
		// We need to check if there's a piece at the destination, and
		// set the move as taking it. Otherwise, we'll trip over some
		// consistency checks that make sure we don't erase pieces.
		//
		dst = MOVE_DST (move);
		piece = PIECE_AT_COORD (game->board, dst);

		if (PIECE_TYPE(piece) != PIECE_NONE)
		{
			assert (PIECE_COLOR(piece) != game->turn);
			if (!is_capture_move(move))
			    move = move_with_capture(move);
		}

		game_move (game, move);
	}

	fclose (f);
	
	return game;
}

// vi: set ts=4 sw=4:
