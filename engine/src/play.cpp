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

namespace wisdom
{
    // the color the computer is playing as
    static const Color comp_player = Color::Black;

    struct InputState
    {
        bool stop_game = false;
        bool show_move_error = false;
        bool pause = false;
        bool unpause = false;
        std::optional<Move> move = std::nullopt;

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

    static std::optional<Game> load_game ()
    {
        std::string input = prompt ("load what file");
        if (input.empty ())
            return std::nullopt;
        return Game::load (input, comp_player);
    }

    static std::optional<Game> load_fen (Game &current_game)
    {
        std::string input = prompt ("FEN game");
        if (input.empty ())
            return std::nullopt;
        try
        {
            FenParser parser { input };
            auto game = parser.build ();
            game.set_computer_player (current_game.get_computer_player ());
            return game;
        }
        catch (FenParser &error)
        {
            return std::nullopt;
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
            result.show_move_error = true;
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
            auto optional_game = load_game ();
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
            result.pause = true;
            return result;
        }
        else if (input == "unpause")
        {
            result.unpause = true;
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
        else if (input == "quit" || input == "exit")
        {
            result.stop_game = true;
            return result;
        }
        else if (input == "analyze")
        {
            load_analysis (game, logger);
            return result;
        }

        result.move = move_parse_optional (input, game.get_current_turn());
        result.show_move_error = true;

        // check the generated move list for this move to see if its valid
        MoveList moves = generate_legal_moves (game.get_board (), game.get_current_turn ());

        for (auto legal_move : moves)
        {
            if (result.move.has_value () && move_equals (legal_move, *result.move))
            {
                result.show_move_error = false;
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
            result.stop_game = true;
            result.show_move_error = true;
            return result;
        }

        if (input[0] == 'y' || input[1] == 'Y')
        {
            std::cout << "Draw accepted!\n";
            result.stop_game = true;
            return result;
        }

        return offer_draw ();
    }

    void play (Color human_player)
    {
        Game game { Color::White, color_invert (human_player) };
        InputState initial_input_state;
        Logger &output = make_standard_logger ();
        InputState input_state = initial_input_state;
        bool paused = false;

        while (true)
        {
            input_state = initial_input_state;
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
                if (input_state.stop_game)
                    break;

                if (input_state.pause)
                {
                    paused = true;
                    continue;
                }
                else if (input_state.unpause)
                {
                    paused = false;
                    continue;
                }
            }
            if (input_state.show_move_error)
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
    Color human_player = Color::White;
    if (argc > 1)
    {
        std::string color_param = argv[1];
        if (color_param == "white")
            human_player = Color::White;
        else if (color_param == "black")
            human_player = Color::Black;
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
