#include "global.hpp"
#include "move.hpp"
#include "board.hpp"
#include "generate.hpp"
#include "search.hpp"
#include "piece.hpp"
#include "check.hpp"
#include "game.hpp"
#include "str.hpp"
#include "logger.hpp"
#include "analytics_sqlite.hpp"
#include "fen_parser.hpp"

#include <iostream>

#include "argh.h"

namespace wisdom
{
    using std::optional;
    using std::nullopt;

    enum class PlayCommand
    {
        None,
        PlayMove,
        ShowError,
        Pause,
        Unpause,
        StopGame,
    };

    struct InputState
    {
        PlayCommand command = PlayCommand::None;
        optional<Move> move = nullopt;

        InputState() = default;
    };

    static void print_available_moves (Game &game)
    {
        MoveList moves = generate_legal_moves (game.get_board (), game.get_current_turn ());

        std::cout << "\nAvailable moves:\n    ";

        int count = 0;
        for (auto move : moves)
        {
            std::cout << "[" << to_string (move) << "] ";
            if (++count % 10 == 0)
                std::cout << "\n" << "    ";
        }

        std::cout << "\n\n";
    }

    static std::string prompt (const std::string &prompt)
    {
        std::string input;
        std::cout << prompt << "? ";

        if (!std::getline (std::cin, input))
            return "";

        return chomp (input);
    }

    static void save_game (const Game &game)
    {
        std::string input = prompt ("save to what file");

        if (input.empty ())
            return;

        game.save (input);
    }

    static optional<Game> load_game (const Game &current_game)
    {
        std::string input = prompt ("load what file");

        if (input.empty ())
            return nullopt;

        return Game::load (input, current_game.get_computer_player ());
    }

    static optional<Game> load_fen (Game &current_game)
    {
        std::string input = prompt ("FEN game");
        if (input.empty ())
            return nullopt;

        try
        {
            FenParser parser { input };
            auto game = parser.build ();
            game.set_computer_player (current_game.get_computer_player ());
            return game;
        }
        catch ([[maybe_unused]] FenParserError &error)
        {
            return nullopt;
        }
    }

    static void load_analysis (Game &game, Logger &logger)
    {
        std::string input = prompt ("store analysis in what file");
        if (input.empty ())
            return;

        game.set_analytics (analysis::make_sqlite_analytics (input, logger));
    }

    static InputState read_move (Game &game, Logger &logger)
    {
        InputState result;
        std::string input;

        std::cout << "(" << wisdom::to_string (game.get_current_turn ()) << ")? ";

        if (!std::getline (std::cin, input))
        {
            result.command = PlayCommand::ShowError;
            return result;
        }

        input = chomp (input);

        if (input == "moves")
        {
            print_available_moves (game);
            return result;
        }
        else if (input == "save")
        {
            save_game (game);
            return result;
        }
        else if (input == "load")
        {
            Color orig_player = game.get_computer_player ();
            auto optional_game = load_game (game);
            if (optional_game.has_value ())
            {
                game = std::move (*optional_game);
                game.set_computer_player (orig_player);
            }
            return result;
        }
        else if (input == "fen")
        {
            Color orig_player = game.get_computer_player ();
            auto optional_game = load_fen (game);
            if (optional_game.has_value ())
            {
                game = std::move (*optional_game);
                game.set_computer_player (orig_player);
            }
            return result;
        }
        else if (input == "pause")
        {
            result.command = PlayCommand::Pause;
            return result;
        }
        else if (input == "unpause")
        {
            result.command = PlayCommand::Unpause;
            return result;
        }
        else if (input == "computer_black")
        {
            game.set_computer_player (Color::Black);
            return result;
        }
        else if (input == "computer_white")
        {
            game.set_computer_player (Color::White);
            return result;
        }
        else if (input == "switch")
        {
            game.set_current_turn (color_invert (game.get_current_turn ()));
            return result;
        }
        else if (input == "quit" || input == "exit")
        {
            result.command = PlayCommand::StopGame;
            return result;
        }
        else if (input == "analyze")
        {
            load_analysis (game, logger);
            return result;
        }

        result.move = move_parse_optional (input, game.get_current_turn());
        result.command = PlayCommand::ShowError;

        // check the generated move list for this move to see if its valid
        MoveList moves = generate_legal_moves (game.get_board (), game.get_current_turn ());

        for (auto legal_move : moves)
        {
            if (result.move.has_value () && move_equals (legal_move, *result.move))
            {
                result.command = PlayCommand::PlayMove;
                break;
            }
        }

        return result;
    }

    static InputState offer_draw ()
    {
        InputState result;
        std::string input;
        std::cout << "Third repetition detected. Would you like a draw? [y/n]\n";

        if (!std::getline (std::cin, input))
        {
            result.command = PlayCommand::StopGame;
            return result;
        }

        if (input[0] == 'y' || input[1] == 'Y')
        {
            std::cout << "Draw accepted!\n";
            result.command = PlayCommand::StopGame;
            return result;
        }

        return offer_draw ();
    }

    void play (Color human_player)
    {
        Game game { Color::White, color_invert (human_player) };
        InputState initial_input_state;
        Logger &output = make_standard_logger ();
        bool paused = false;

        while (true)
        {
            InputState input_state = initial_input_state;
            game.get_board ().print ();

            if (is_checkmated (game.get_board (), game.get_current_turn()))
            {
                std::cout << to_string (color_invert (game.get_current_turn())) << " wins the game.\n";
                return;
            }

            if (game.get_history().is_third_repetition (game.get_board()))
            {
                input_state = offer_draw ();
                continue;
            }

            if (History::is_fifty_move_repetition (game.get_board ()))
            {
                std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
                break;
            }

            if (!paused && game.is_computer_turn ())
            {
                auto optional_move = game.find_best_move (output);
                if (!optional_move.has_value ())
                {
                    std::cout << "\nCouldn't find move!\n";
                    break;
                }

                input_state.move = optional_move;
                if (input_state.move.has_value ())
                    std::cout << "move selected: [" << to_string (*input_state.move) << "]\n";
            }
            else
            {
                input_state = read_move (game, output);

                if (input_state.command == PlayCommand::StopGame)
                    break;

                if (input_state.command == PlayCommand::Pause)
                {
                    paused = true;
                    continue;
                }
                else if (input_state.command == PlayCommand::Unpause)
                {
                    paused = false;
                    continue;
                }
            }

            if (input_state.command == PlayCommand::ShowError)
            {
                std::cout << "\nInvalid move\n\n";
                continue;
            }
            
            if (input_state.move.has_value ())
                game.move (*input_state.move);
        }
    }
}

using wisdom::Color;

int main (int argc, char **argv)
{
    argh::parser cmdline { argv, argh::parser::PREFER_FLAG_FOR_UNREG_OPTION };

    Color human_player = Color::White;

    if (cmdline("--player"))
    {
        std::string player_str = cmdline("--player").str();
        std::cout << player_str << "\n";

        if (player_str == "White")
            human_player = Color::White;
        else if (player_str == "Black")
            human_player = Color::Black;
        else
        {
            std::cerr << "Invalid player" << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Overriding player to " << wisdom::to_string (human_player) << "\n";
    }

    try
    {
        wisdom::play (human_player);
    }
    catch (const wisdom::AssertionError &e)
    {
        std::cerr << e.message() << "\n";
        std::cerr << " for " << e.extra_info() << " at " << e.file() << ":" << e.line() << "\n";
        std::terminate ();
    }
    catch (const wisdom::Error &e)
    {
        std::cerr << "Uncaught Error!" << "\n";
        std::cerr << e.message() << "\n";
        std::cerr << e.extra_info() << "\n";
        std::terminate ();
    }

    return 0;
}
