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

namespace wisdom::ui::console
{
    using namespace wisdom;

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

    static auto human_wants_draw (const string& msg) -> bool
    {
        InputState result;
        string input;

        while (toupper(input[0]) != 'Y' && toupper(input[0]) != 'N')
        {
            std::cout << msg;

            if (!std::getline(std::cin, input))
                continue;
        }

        return (input[0] == 'y' || input[1] == 'Y');
    }

    static auto player_wants_draw (const string& msg, Player player, Color who, Game& game, bool asked_human) -> DrawStatus
    {
        if (player == Player::Human)
        {
            if (asked_human)
                return DrawStatus::Declined;

            return human_wants_draw (msg) ? DrawStatus::Accepted : DrawStatus::Declined;
        }
        return game.computerWantsDraw (who) ? DrawStatus::Accepted : DrawStatus::Declined;
    }

    // After the third repetition, either player may request a draw.
    static auto determine_if_drawn (const string& msg, InputState input_state, Game& game)
        -> std::pair<DrawStatus, DrawStatus>
    {
        auto white_player = game.getPlayer (Color::White);

        auto white_wants_draw = player_wants_draw (
            msg, white_player, Color::White, game, false
        );
        bool asked_human = white_player == Player::Human;
        auto black_wants_draw = player_wants_draw (
            msg, game.getPlayer (Color::Black), Color::Black, game, asked_human
        );

        return { white_wants_draw, black_wants_draw };
    }

    class ConsoleGameStatusManager : public GameStatusUpdate
    {
    private:
        InputState my_input_state {};
        Game my_game {};

    public:
        ConsoleGameStatusManager() = default;
        ~ConsoleGameStatusManager() override = default;

        [[nodiscard]] auto get_input_state() & -> InputState
        {
            return my_input_state;
        }

        void set_input_state (const InputState& new_state)
        {
            my_input_state = new_state;
        }

        [[nodiscard]] auto get_game() & -> Game&
        {
            return my_game;
        }
        void get_game() && = delete;

        void handle_draw (const string& msg, ProposedDrawType draw_type)
        {
            // Recursively (one-level deep) update the status again.
            auto draw_pair = determine_if_drawn (msg, my_input_state, my_game);
            my_game.setProposedDrawStatus (ProposedDrawType::ThreeFoldRepetition, draw_pair);
            return update (my_game.status());
        }

        void checkmate() override
        {
            std::cout << asString (colorInvert (my_game.getCurrentTurn())) << " wins the game.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void stalemate() override
        {
            my_input_state.command = PlayCommand::StopGame;
        }

        void insufficientMaterial() override
        {
            std::cout << "Draw: Insufficient material.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void thirdRepetitionDrawReached() override
        {
            std::string message = "Threefold repetition detected. Would you like a draw? [y/n]\n";
            handle_draw (message, ProposedDrawType::ThreeFoldRepetition);
        }

        void thirdRepetitionDrawAccepted() override
        {
            std::cout << "Draw: threefold repetition and at least one of the players wants a draw.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void fifthRepetitionDraw() override
        {
            std::cout << "Draw: same position repeated five times.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void fiftyMovesWithoutProgressReached() override
        {
            std::string message = "Fifty moves without progress detected. Would you like a draw? [y/n]\n";
            handle_draw (message, ProposedDrawType::FiftyMovesWithoutProgress);
        }

        void fiftyMovesWithoutProgressAccepted() override
        {
            std::cout << "Draw: Fifty moves without a capture or pawn move and "
                      << "at least one player wants a draw.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void seventyFiveMovesWithNoProgress() override
        {
            std::cout << "Draw: Seventy five moves without a capture or pawn move.\n";
            my_input_state.command = PlayCommand::StopGame;
        }
    };

    static void print_available_moves (Game& game, MoveGenerator& generator)
    {
        MoveList moves = generator.generateLegalMoves (game.getBoard(), game.getCurrentTurn());

        std::cout << "\nAvailable moves:\n    ";

        int count = 0;
        for (auto move : moves)
        {
            std::cout << "[" << asString (move) << "] ";
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

        return Game::load (input, current_game.getPlayers());
    }

    static optional<Game> load_fen (const Game& current_game)
    {
        string input = prompt ("FEN game");
        if (input.empty ())
            return nullopt;

        try
        {
            FenParser parser { input };
            auto game = parser.build ();
            game.setPlayers (current_game.getPlayers());
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

        std::cout << "(" << wisdom::asString (game.getCurrentTurn()) << ")? ";

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
            auto orig_players = game.getPlayers();
            auto optional_game = load_game (game);
            if (optional_game.has_value ())
            {
                game = std::move (*optional_game);
                game.setPlayers (orig_players);
            }
            return result;
        }
        else if (input == "fen")
        {
            auto orig_players = game.getPlayers();
            auto optional_game = load_fen (game);
            if (optional_game.has_value ())
            {
                game = std::move (*optional_game);
                game.setPlayers (orig_players);
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
                game.setMaxDepth (*max_depth);
            return result;
        }
        else if (input == "timeout")
        {
            optional<int> search_timeout = read_int ("Search Timeout");
            if (search_timeout.has_value ())
                game.setSearchTimeout (chrono::seconds { *search_timeout });
            return result;
        }
        else if (input == "computer_black")
        {
            game.setBlackPlayer (Player::ChessEngine);
            return result;
        }
        else if (input == "computer_white")
        {
            game.setWhitePlayer (Player::ChessEngine);
            return result;
        }
        else if (input == "human_white")
        {
            game.setWhitePlayer (Player::Human);
            return result;
        }
        else if (input == "human_black")
        {
            game.setBlackPlayer (Player::Human);
            return result;
        }
        else if (input == "switch")
        {
            game.setCurrentTurn (colorInvert (game.getCurrentTurn()));
            return result;
        }
        else if (input == "quit" || input == "exit")
        {
            result.command = PlayCommand::StopGame;
            return result;
        }

        result.move = moveParseOptional (input, game.getCurrentTurn());
        result.command = PlayCommand::ShowError;

        // check the generated move list for this move to see if its valid
        MoveList moves = move_generator.generateLegalMoves (game.getBoard(), game.getCurrentTurn());

        for (auto legal_move : moves)
        {
            if (result.move.has_value () && moveEquals (legal_move, *result.move))
            {
                result.command = PlayCommand::PlayMove;
                break;
            }
        }

        return result;
    }

    void play ()
    {
        ConsoleGameStatusManager game_status_manager {};

        InputState initial_input_state;
        auto output = makeStandardLogger();
        bool paused = false;
        MoveGenerator move_generator;

        while (true)
        {
            auto& game = game_status_manager.get_game ();
            game_status_manager.set_input_state (initial_input_state);

            game.getBoard().print();

            game_status_manager.update (game.status());
            auto input_state = game_status_manager.get_input_state();

            if (input_state.command == PlayCommand::StopGame)
                break;

            if (!paused && game.getCurrentPlayer() == Player::ChessEngine)
            {
                auto optional_move = game.findBestMove (*output);
                if (!optional_move.has_value ())
                {
                    std::cout << "\nCouldn't find move!\n";
                    break;
                }

                input_state.move = optional_move;
                if (input_state.move.has_value ())
                    std::cout << "move selected: [" << asString (*input_state.move) << "]\n";
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
