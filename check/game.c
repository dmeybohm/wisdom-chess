#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "piece.h"
#include "board.h"
#include "move.h"
#include "move_tree.h"
#include "str.h"
#include "game.h"

struct game *game_new (color_t turn, color_t computer_player)
{
	struct game *game;

	game = malloc (sizeof (struct game));
	assert (game != NULL);

	game->board = board_new ();
	assert (game->board != NULL);

	game->history = NULL;

	/* if 'turn' is something bogus, use white */
	if (is_color_invalid (turn))
		turn = COLOR_WHITE;

	/* if 'computer_player' is bogus, use black */
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

void game_move (struct game *game, move_t *move)
{
	/* add this move to the history */
	game->history = move_tree_new (game->history, *move);

	/* do the move */
	do_move (game->board, game->turn, move);

	/* take our turn */
	game->turn = color_invert (game->turn);
}

int game_save (struct game *game)
{
	FILE *out;
	char buf[128];
	move_tree_t *history;

	printf ("save to what file? ");
	fflush (stdout);

	if (fgets (buf, sizeof (buf)-1, stdin) == NULL)
		return 0;

	chomp (buf);

	if (!(out = fopen (buf, "w+")))
	{
		printf ("error opening file: %s", strerror (errno));
		return 0;
	}

	history = game->history;

	while (!is_null_move (history->move))
	{
		fprintf (out, "%s\n", move_str (history->move));
		history = history->parent;
	}

	fclose (out);

	return 1; /* need to check for failure here */
}

struct game *game_load (const char *file, color_t player)
{
	struct game *game;
	char buf[128];
	FILE *f;
	move_t move;

	if (!(f = fopen (file, "r")))
		return NULL;

	game = game_new (player, COLOR_NONE);

	while (fgets (buf, sizeof (buf) - 1, f) != NULL)
	{
		if (!move_parse (buf, game->turn, &move))
		{
			printf ("Error parsing game file %s: invalid move \"%s\"",
					file, buf);
			game_free (game);
			return NULL;
		}

		game_move (game, &move);
	}

	fclose (f);
	
	return game;
}

/* vi: set ts=4 sw=4: */
