#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <iostream>
#include <memory>
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

static void print_available_moves (std::unique_ptr<game> &game)
{
	move_list_t moves = generate_legal_moves (game->board, game->turn);

	printf ("\nAvailable moves: ");

	for (auto move : moves)
		printf ("[%s] ", move_str (move));

	printf("\n\n");
}

struct input_state_t
{
    bool ok;
    bool good;
    bool skip;
    move_t move;
};

struct read_move_state_t
{
    input_state_t state;
    std::unique_ptr<game> new_game;
};

static input_state_t initial_input_state {
        .ok = true,
        .good = true,
        .skip = false,
        .move = null_move
};

static read_move_state_t read_move (std::unique_ptr<game> &game)
{
    read_move_state_t result {
        .state = initial_input_state,
        .new_game = nullptr
    } ;
    std::string        input;

	std::cout << "move? ";

	if (!std::getline(std::cin, input))
	    return result;

	input = chomp (input);

	if (input == "moves") {
		print_available_moves (game);
        result.state.skip = true;
		return result;
	}
	else if (input == "save")
	{
		game->save();
        result.state.skip = true;
		return result;
	}
	else if (input == "load")
	{
        result.state.skip = true;
        result.new_game = std::move (game::load (comp_player));
		return result;
	}
	else if (input == "fenload")
    {
	    // todo
	    result.state.skip = true;
    }

    result.state.move = move_parse (input.c_str(), game->turn);

    result.state.good = false;

	// check the generated move list for this move to see if its valid
	move_list_t moves = generate_legal_moves (game->board, game->turn);

	for (auto legal_move : moves)
	{
        if (move_equals (legal_move, result.state.move))
        {
            result.state.good = true;
            break;
        }
    }

	return result;
}

int main (int argc, char **argv)
{
    auto game = std::make_unique<struct game> (COLOR_WHITE, comp_player);
    input_state_t input_state { initial_input_state };

	while (input_state.ok)
	{
        input_state = initial_input_state;
		board_print (game->board);

		if (is_checkmated (game->board, game->turn))
		{
			printf ("%s wins the game\n", game->turn == COLOR_WHITE ? "Black" : 
			        "White");
			printf ("Save the game? ");
			return 0;
		}

		if (game->turn != game->player)
		{
			read_move_state_t read_move_state = read_move (game);
			input_state = read_move_state.state;

            if (read_move_state.new_game != nullptr)
                game = std::move(read_move_state.new_game);
		}
		else
		{
			input_state.move = find_best_move (game->board, game->player, game->history);

			printf ("move selected: [%s]\n", move_str (input_state.move));
			input_state.good = true;
			input_state.ok = true;
		}

		if (input_state.skip)
			continue;

		if (input_state.good)
			game->move (input_state.move);
		else if (input_state.ok)
			printf("\nInvalid move\n\n");
	}

	return 0;
}

