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
#include "fen_parser.hpp"

#include <iostream>

namespace wisdom
{
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

    static void print_available_moves (Game& game, MoveGenerator& generator)
    {
        MoveList moves = generator.generate_legal_moves (game.get_board (), game.get_current_turn ());

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

    static string prompt (const string& prompt)
    {
        string input;
        std::cout << prompt << "? ";

        if (!std::getline (std::cin, input))
            return "";

        return chomp (input);
    }

    static void save_game (const Game& game)
    {
        string input = prompt ("save to what file");

        if (input.empty ())
            return;

        game.save (input);
    }

    static optional<Game> load_game (const Game& current_game)
    {
        string input = prompt ("load what file");

        if (input.empty ())
            return nullopt;

        return Game::load (input, current_game.get_players ());
    }

    static optional<Game> load_fen (Game& current_game)
    {
        string input = prompt ("FEN game");
        if (input.empty ())
            return nullopt;

        try
        {
            FenParser parser { input };
            auto game = parser.build ();
            game.set_players (current_game.get_players ());
            return game;
        }
        catch ([[maybe_unused]] FenParserError &error)
        {
            return nullopt;
        }
    }

    static optional<int> read_int (const std::string& prompt_value)
    {
        string input = prompt (prompt_value);
        if (input.empty ())
            return nullopt;

        try {
            return std::stoi(input);
        } catch (std::invalid_argument& e) {
            return nullopt;
        }
    }

    static InputState read_move (Game& game, MoveGenerator& move_generator)
    {
        InputState result;
        string input;

        std::cout << "(" << wisdom::to_string (game.get_current_turn ()) << ")? ";

        if (!std::getline (std::cin, input))
        {
            result.command = PlayCommand::StopGame;
            return result;
        }

        input = chomp (input);

        if (input == "moves")
        {
            print_available_moves (game, move_generator);
            return result;
        }
        else if (input == "save")
        {
            save_game (game);
            return result;
        }
        else if (input == "load")
        {
            auto orig_players = game.get_players ();
            auto optional_game = load_game (game);
            if (optional_game.has_value ())
            {
                game = std::move (*optional_game);
                game.set_players (orig_players);
            }
            return result;
        }
        else if (input == "fen")
        {
            auto orig_players = game.get_players ();
            auto optional_game = load_fen (game);
            if (optional_game.has_value ())
            {
                game = std::move (*optional_game);
                game.set_players (orig_players);
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
        else if (input == "maxdepth")
        {
            optional<int> max_depth = read_int ("Max depth");
            if (max_depth.has_value ())
                game.set_max_depth (*max_depth);
            return result;
        }
        else if (input == "timeout")
        {
            optional<int> search_timeout = read_int ("Search Timeout");
            if (search_timeout.has_value ())
                game.set_search_timeout (chrono::seconds { *search_timeout });
            return result;
        }
        else if (input == "computer_black")
        {
            game.set_black_player (Player::ChessEngine);
            return result;
        }
        else if (input == "computer_white")
        {
            game.set_white_player (Player::ChessEngine);
            return result;
        }
        else if (input == "human_white")
        {
            game.set_white_player (Player::Human);
            return result;
        }
        else if (input == "human_black")
        {
            game.set_black_player (Player::Human);
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

        result.move = move_parse_optional (input, game.get_current_turn());
        result.command = PlayCommand::ShowError;

        // check the generated move list for this move to see if its valid
        MoveList moves = move_generator.generate_legal_moves (game.get_board (), game.get_current_turn ());

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

    static auto human_wants_draw () -> bool
    {
        InputState result;
        string input;

        while (toupper(input[0]) != 'Y' && toupper(input[0]) != 'N')
        {
            std::cout << "Threefold repetition detected. Would you like a draw? [y/n]\n";

            if (!std::getline(std::cin, input))
                continue;
        }

        return (input[0] == 'y' || input[1] == 'Y');
    }

    static auto player_wants_draw (Player player, Color who, Game& game, bool asked_human) -> DrawStatus
    {
        if (player == Player::Human)
        {
            if (asked_human)
                return DrawStatus::Declined;

            return human_wants_draw () ? DrawStatus::Accepted : DrawStatus::Declined;
        }
        return game.computer_wants_draw (who) ? DrawStatus::Accepted : DrawStatus::Declined;
    }

    // After the third repetition, either player may request a draw.
    static auto determine_if_drawn (InputState input_state, Game& game) -> std::pair<DrawStatus, DrawStatus>
    {
        auto white_player = game.get_player (Color::White);

        auto white_wants_draw = player_wants_draw (
                white_player, Color::White, game, false
        );
        bool asked_human = white_player == Player::Human;
        auto black_wants_draw = player_wants_draw (
                game.get_player (Color::Black), Color::Black, game, asked_human
        );

        return { white_wants_draw, black_wants_draw };
    }

    auto update_game_status (InputState input_state, Game& game) -> InputState // NOLINT(misc-no-recursion)
    {
        auto status = game.status ();

        switch (status)
        {
        case GameStatus::Playing:
            break;

        case GameStatus::Checkmate:
            std::cout << to_string (color_invert (game.get_current_turn())) << " wins the game.\n";
            input_state.command = PlayCommand::StopGame;
            break;

        case GameStatus::Stalemate:
            input_state.command = PlayCommand::StopGame;
            break;

        case GameStatus::ThreefoldRepetitionReached: {
            auto draw_pair = determine_if_drawn (input_state, game);
            game.set_proposed_draw_status (
                    ProposedDrawType::ThreeFoldRepetition,
                    draw_pair
            );
            // Recursively (one-level deep) update the status again.
            return update_game_status (input_state, game);
        }

        case GameStatus::ThreefoldRepetitionAccepted:
            std::cout << "Draw: threefold repetition and at least one of the players wants a draw.\n";
            input_state.command = PlayCommand::StopGame;
            break;

        case GameStatus::FivefoldRepetitionDraw:
            std::cout << "Draw: same position repeated five times.\n";
            input_state.command = PlayCommand::StopGame;
            break;

        case GameStatus::FiftyMovesWithoutProgressReached: {
            auto draw_pair = determine_if_drawn (input_state, game);
            game.set_proposed_draw_status (
                    ProposedDrawType::FiftyMovesWithoutProgress,
                    draw_pair
            );
            // Recursively (one-level deep) update the status again.
            return update_game_status (input_state, game);
        }

        case GameStatus::FiftyMovesWithoutProgressAccepted:
            std::cout << "Draw: Fifty moves without a capture or pawn move and "
                        << "at least one player wants a draw.\n";
            input_state.command = PlayCommand::StopGame;
            break;

        case GameStatus::SeventyFiveMovesWithoutProgressDraw:
            std::cout << "Draw: Seventy five moves without a capture or pawn move.\n";
            input_state.command = PlayCommand::StopGame;
            break;

        case GameStatus::InsufficientMaterialDraw:
            std::cout << "Draw: Insufficient material.\n";
            input_state.command = PlayCommand::StopGame;
            break;
        }

        return input_state;
    }

    void play ()
    {
        Game game;
        InputState initial_input_state;
        auto output = make_standard_logger ();
        bool paused = false;
        MoveGenerator move_generator;

        while (true)
        {
            InputState input_state = initial_input_state;
            game.get_board ().print ();

            input_state = update_game_status (input_state, game);

            if (input_state.command == PlayCommand::StopGame)
                break;

            if (!paused && game.get_current_player () == Player::ChessEngine)
            {
                auto optional_move = game.find_best_move (*output);
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
                input_state = read_move (game, move_generator);

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
