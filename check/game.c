#include <stdio.h>
#include <stdlib.h>

#include "piece.h"
#include "board.h"
#include "move_tree.h"

#include "game.h"

struct game *game_new (color_t turn)
{
	struct game *game;

	game = malloc (sizeof (struct game));
	assert (game != NULL);

	game->board = board_new ();
	assert (game->board != NULL);

	game->history = NULL;

	/* if 'turn' is something bogus, use white */
	if (turn != COLOR_WHITE && turn != COLOR_BLACK)
		turn = COLOR_WHITE;

	game->turn = turn;

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
/* vi: set ts=4 sw=4: */
