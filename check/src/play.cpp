#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <iostream>

#include "move.h"
#include "board.h"
#include "generate.h"
#include "search.h"
#include "piece.h"
#include "check.h"
#include "game.h"
#include "str.h"

// the color the computer is playing as
static const enum color comp_player = COLOR_BLACK;

static void print_available_moves (struct game *game)
{
	move_list_t moves = generate_legal_moves (game->board, game->turn);

	printf ("\nAvailable moves: ");

	for (auto move : moves)
		printf ("[%s] ", move_str (move));

	printf("\n\n");
}

static int read_move (struct game **g_out, int *good, int *skip, move_t *move)
{
	char         buf[128];
#if 0
	piece_t src_piece, dst_piece;
#endif
	move_list_t moves;
	struct game        *game  = *g_out;

	printf ("move? ");
	fflush (stdout);

	if (fgets (buf, sizeof(buf)-1, stdin) == nullptr)
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
	else if (!strncmp (buf, "load", strlen ("load")))
	{
		struct game *new_game;

		*skip = 1;

		if (!(new_game = game_load (comp_player)))
			return 1;

		// this breaks because we have a null move atop the history tree
		//game_free (game);

		*g_out = new_game;
		return 1;
	}

	*move = move_parse (buf, game->turn);

	*good = 0;

	// check the generated move list for this move to see if its valid
	moves = generate_legal_moves (game->board, game->turn);

	for (auto legal_move : moves)
	{
        if (move_equals (legal_move, *move))
        {
            *good = 1;
            break;
        }
    }

	return 1;
}

int main (int argc, char **argv)
{
	struct game  *game;
	int           ok = 1, good;
	int           skip;
	move_t        move = move_null();

	game = game_new (COLOR_WHITE, comp_player);

	while (ok)
	{
		skip = 0;

		board_print (game->board);

		if (is_checkmated (game->board, game->turn))
		{
			printf ("%s wins the game\n", game->turn == COLOR_WHITE ? "Black" : 
			        "White");
			game_free (game);
			return 0;
		}

		if (game->turn != game->player)
		{
			ok = read_move (&game, &good, &skip, &move);
		}
		else
		{
			move = find_best_move (game->board, game->player, game->history);

			printf ("move selected: [%s]\n", move_str (move));
			good = 1;
			ok = 1;
		}

		if (skip)
			continue;

		if (good)
			game_move (game, move);
		else if (ok)
			printf("\nInvalid move\n\n");
	}

	return 0;
}

// vi: set ts=4 sw=4:
