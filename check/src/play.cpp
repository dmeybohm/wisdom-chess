#include <cstdio>
#include <iostream>
#include <memory>

#include "move.h"
#include "board.h"
#include "generate.h"
#include "search.h"
#include "piece.h"
#include "check.h"
#include "game.h"
#include "str.h"

// the color the computer is playing as
static const Color comp_player = Color::Black;

static void print_available_moves (game &game)
{
	move_list_t moves = generate_legal_moves (game.board, game.turn);

	std::cout << "\nAvailable moves: ";

	for (auto move : moves)
	    std::cout << "[" << to_string(move) << "] ";

	std::cout <<"\n\n";
}

struct input_state_t
{
    bool ok;
    bool good;
    bool skip;
    move_t move;
};

static input_state_t initial_input_state {
        .ok = true,
        .good = true,
        .skip = false,
        .move = null_move
};

static input_state_t read_move (game &game)
{
    input_state_t result = initial_input_state;
    std::string   input;

	std::cout << "move? ";

	if (!std::getline(std::cin, input))
	    return result;

	input = chomp (input);

	if (input == "moves") {
		print_available_moves (game);
        result.skip = true;
		return result;
	}
	else if (input == "save")
	{
		game.save();
        result.skip = true;
		return result;
	}
	else if (input == "load")
	{
        result.skip = true;
        auto optional_game = game::load (comp_player);
        if (optional_game.has_value())
            game = *optional_game;
		return result;
	}
	else if (input == "fenload")
    {
	    // todo
	    result.skip = true;
    }

    result.move = move_parse (input.c_str(), game.turn);

    result.good = false;

	// check the generated move list for this move to see if its valid
	move_list_t moves = generate_legal_moves (game.board, game.turn);

	for (auto legal_move : moves)
	{
        if (move_equals (legal_move, result.move))
        {
            result.good = true;
            break;
        }
    }

	return result;
}

int main (int argc, char **argv)
{
    struct game game { Color::White, comp_player };
    input_state_t input_state { initial_input_state };

	while (input_state.ok)
	{
        input_state = initial_input_state;
		game.board.print();

		if (is_checkmated (game.board, game.turn))
		{
			printf ("%s wins the game\n", game.turn == Color::White ? "Black" :
			        "White");
			printf ("Save the game? ");
			return 0;
		}

		if (game.turn != game.player)
		{
            input_state = read_move (game);
		}
		else
		{
			input_state.move = find_best_move (game.board, game.player, game.history);

			std::cout << "move selected: [" << to_string (input_state.move) << "]\n";
			input_state.good = true;
			input_state.ok = true;
		}

		if (input_state.skip)
			continue;

		if (input_state.good && !is_null_move(input_state.move))
			game.move (input_state.move);
		else
            std::cout << "\nInvalid move\n\n";
	}

	return 0;
}

