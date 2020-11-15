#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <iostream>

#include "piece.h"
#include "board.h"
#include "move.h"
#include "move_tree.h"
#include "str.h"
#include "game.h"
#include "board_check.h"

void game::move (move_t move)
{
	// add this move to the history
	history.push_back (move);

	// do the move
    do_move (board, turn, move);

	// take our turn
	turn = color_invert (turn);
}

static std::string prompt (const char *prompt)
{
    std::string input;
    std::cout << prompt << "? ";

    if (!std::getline(std::cin, input))
        return "";

    return chomp (input);
}

bool game::save ()
{
	std::string input = prompt ("save to what file");
	if (input.size() == 0)
		return false;

	history.save(input);
	return true; // need to check for failure here
}

std::unique_ptr<game> game::load (enum color player)
{
	char file[128];
	char buf[128];
	FILE *f;
	move_t move;

	std::string input = prompt ("load what file");
	if (input.size() == 0)
		return nullptr;

	if (!(f = fopen (file, "r")))
	{
		printf ("Couldn't open %s: %s\n", file, strerror (errno));
		return nullptr;
	}

    auto result = std::make_unique<game>(COLOR_NONE, player);

    while (fgets (buf, sizeof (buf) - 1, f) != nullptr)
	{
		coord_t dst;
		piece_t piece;

		if (!strncasecmp (buf, "stop", strlen ("stop")))
			break;

		move = move_parse (buf, result->turn);
		if (is_null_move(move))
		{
			printf ("Error parsing game file %s: invalid move \"%s\"",
					file, buf);
			return nullptr;
		}

		//
		// We need to check if there's a piece at the destination, and
		// set the move as taking it. Otherwise, we'll trip over some
		// consistency checks that make sure we don't erase pieces.
		//
		dst = MOVE_DST(move);
		piece = PIECE_AT_COORD (result->board, dst);

		if (PIECE_TYPE(piece) != PIECE_NONE)
		{
			assert (PIECE_COLOR(piece) != result->turn);

			// for historical reasons, we automatically convert to capture move
			// here. but should probably throw an exception instead.
			if (!is_capture_move(move))
			    move = move_with_capture(move);
		}

		result->move (move);
	}

	fclose (f);
	
	return result;
}
