#include <iostream>
#include <memory>

#include "move.hpp"
#include "board.hpp"
#include "generate.hpp"
#include "search.hpp"
#include "piece.hpp"
#include "check.hpp"
#include "game.hpp"
#include "str.hpp"
#include "output.hpp"

namespace wisdom
{
    // the color the computer is playing as
    static const Color comp_player = Color::Black;

    static void print_available_moves (Game &game)
    {
        MoveList moves = generate_legal_moves (game.board, game.turn);

        std::cout << "\nAvailable moves: ";

        for (auto move : moves)
            std::cout << "[" << to_string (move) << "] ";

        std::cout << "\n\n";
    }

    struct InputState
    {
        bool ok;
        bool good;
        bool skip;
        std::optional<Move> move;

        static InputState from_initial() noexcept
        {
            return InputState {
                .ok = true,
                .good = true,
                .skip = false,
                .move = {}
            };
        }
    };

    static InputState read_move (Game &game)
    {
        InputState result = InputState::from_initial ();
        std::string input;

        std::cout << "move? ";

        if (!std::getline (std::cin, input))
        {
            result.ok = false;
            return result;
        }

        input = chomp (input);

        if (input == "moves")
        {
            print_available_moves (game);
            result.skip = true;
            return result;
        }
        else if (input == "save")
        {
            game.save ();
            result.skip = true;
            return result;
        }
        else if (input == "load")
        {
            result.skip = true;
            auto optional_game = Game::load (comp_player);
            if (optional_game.has_value ())
                game = *optional_game;
            return result;
        }
        else if (input == "quit" || input == "exit")
        {
            result.ok = false;
            return result;
        }

        result.move = move_parse_optional (input, game.turn);
        result.good = false;

        // check the generated move list for this move to see if its valid
        MoveList moves = generate_legal_moves (game.board, game.turn);

        for (auto legal_move : moves)
        {
            if (result.move.has_value () && move_equals (legal_move, result.move.value ()))
            {
                result.good = true;
                break;
            }
        }

        return result;
    }

    static InputState offer_draw ()
    {
        InputState result = InputState::from_initial ();
        std::string input;
        std::cout << "Third repetition detected. Would you like a draw? [y/n]\n";

        if (!std::getline (std::cin, input))
        {
            result.ok = result.good = false;
            return result;
        }

        if (input[0] == 'y' || input[1] == 'Y')
        {
            std::cout << "Draw accepted!\n";
            result.ok = false;
            return result;
        }

        return offer_draw ();
    }
}

using namespace wisdom;

int main (int argc, char **argv)
{
    Game game { Color::White, comp_player };
    InputState initial_input_state = InputState::from_initial ();
    wisdom::StandardOutput output;
    InputState input_state = initial_input_state;

    while (input_state.ok)
    {
        input_state = initial_input_state;
        game.board.print ();

        if (is_checkmated (game.board, game.turn))
        {
            std::cout << to_string (color_invert (game.turn)) << " wins the game.\n";
            return 0;
        }

        if (game.history.is_third_repetition (game.board))
        {
            input_state = offer_draw ();
            continue;
        }

        if (History::is_fifty_move_repetition (game.board))
        {
            std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
            break;
        }

        if (game.turn != game.player)
        {
            input_state = read_move (game);
            if (!input_state.ok)
                break;
        }
        else
        {
            auto optional_move = find_best_move (game.board, game.player, output, game.history);
            if (!optional_move.has_value ())
            {
                std::cout << "\nCouldn't find move!\n";
                break;
            }

            input_state.move = optional_move;
            std::cout << "move selected: [" << to_string (input_state.move.value()) << "]\n";
            input_state.good = true;
            input_state.ok = true;
        }

        if (input_state.skip)
            continue;

        if (input_state.good && input_state.move.has_value ())
            game.move (input_state.move.value ());
        else
            std::cout << "\nInvalid move\n\n";
    }

    return 0;
}
