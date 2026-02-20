#include <variant>
#include <iostream>
#include <memory>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"

namespace wisdom::ui::console
{
    using namespace wisdom;
    using std::holds_alternative;
    using std::get;
    using std::string;

    namespace PlayCommand
    {
        struct None {};
        struct Help {};

        struct ShowError
        {
            string message;

            explicit ShowError (string new_message)
                    : message { std::move (new_message) }
            {}
        };

        struct ShowInfo
        {
            string message;

            explicit ShowInfo (string new_message)
                : message { std::move (new_message) }
            {}
        };

        struct Pause {};
        struct Unpause {};

        struct StopGame
        {
            bool show_final_position = true;

            StopGame() = default;

            static auto fromShowFinalPosition (bool new_show_final_position) -> StopGame
            {
                auto result = StopGame {};
                result.show_final_position = new_show_final_position;
                return result;
            }
        };

        struct SaveGame
        {
            string file_path;

            explicit SaveGame (string new_file_path)
                : file_path { std::move (new_file_path) }
            {}
        };

        struct LoadNewGame
        {
            Game new_game;

            explicit LoadNewGame (Game game)
                : new_game { std::move (game) }
            {
            }
        };

        struct PrintAvailableMoves {};
        struct SwitchSides {};

        struct SetSearchTimeout
        {
            chrono::seconds seconds;

            explicit SetSearchTimeout (chrono::seconds new_seconds)
                : seconds { new_seconds }
            {}
        };

        struct SetMaxDepth
        {
            int max_depth;

            explicit SetMaxDepth (int new_max_depth)
                : max_depth { new_max_depth }
            {}
        };

        struct SetPlayer
        {
            Color side;
            Player player_type;

            explicit SetPlayer (Color new_side, Player new_player)
                : side { new_side }
                , player_type { new_player }
            {}
        };

        struct PlayMove
        {
            Move move;

            explicit PlayMove (Move new_move)
                : move { new_move }
            {}
        };

        using AnyCommand = std::variant<
            None,
            Help,
            ShowError,
            ShowInfo,
            Pause,
            Unpause,
            StopGame,
            SaveGame,
            LoadNewGame,
            PrintAvailableMoves,
            SwitchSides,
            SetPlayer,
            SetSearchTimeout,
            SetMaxDepth,
            PlayMove
        >;
    }

    class ConsoleGame : public ui::GameViewModelBase
    {
    private:
        Game my_game;
        bool quit = false;
        bool paused = false;
        bool show_final_position = true;

    protected:
        [[nodiscard]] auto getGame() -> observer_ptr<Game> override
        {
            return &my_game;
        }

        [[nodiscard]] auto getGame() const -> observer_ptr<const Game> override
        {
            return &my_game;
        }

        [[nodiscard]] auto
        formatBold (const std::string& text) const
            -> std::string override
        {
            return text;
        }

        void onGameOverStatusChanged() override
        {
            auto status = gameOverStatus();
            if (!status.empty())
            {
                std::cout << status << "\n";
                quit = true;
            }
        }

        void onInCheckChanged() override
        {
            if (inCheck())
            {
                std::cout << "Check!\n";
            }
        }

    public:
        ConsoleGame() : my_game { Game::createStandardGame() }
        {
        }

        void play();

        void setQuit (bool new_quit_game)
        {
            quit = new_quit_game;
        }

        void setPaused (bool new_paused)
        {
            paused = new_paused;
        }

        void setShowFinalPosition (bool new_final_position)
        {
            show_final_position = new_final_position;
        }

        static auto humanWantsDraw (const string& msg) -> bool
        {
            string input;

            while (toupper (input[0]) != 'Y' && toupper (input[0]) != 'N')
            {
                std::cout << msg;

                if (!std::getline (std::cin, input))
                    continue;
            }

            return (input[0] == 'y' || input[1] == 'Y');
        }

        auto playerWantsDraw (const string& msg, Player player, Color who, bool asked_human)
            -> DrawStatus
        {
            if (player == Player::Human)
            {
                if (asked_human)
                    return DrawStatus::Declined;

                return humanWantsDraw (msg) ? DrawStatus::Accepted : DrawStatus::Declined;
            }
            return my_game.computerWantsDraw (who) ? DrawStatus::Accepted : DrawStatus::Declined;
        }

        // After the third repetition, either player may request a draw.
        auto determineIfDrawn (const string& msg) -> std::pair<DrawStatus, DrawStatus>
        {
            auto white_player = my_game.getPlayer (Color::White);

            auto white_wants_draw = playerWantsDraw (msg, white_player, Color::White, false);
            bool asked_human = white_player == Player::Human;
            auto black_wants_draw
                = playerWantsDraw (msg, my_game.getPlayer (Color::Black), Color::Black, asked_human);

            return { white_wants_draw, black_wants_draw };
        }

        // Handle draw proposals synchronously before calling updateDisplayedGameState().
        void handleDrawProposals()
        {
            auto status = my_game.status();

            if (status == GameStatus::ThreefoldRepetitionReached)
            {
                string message = "Threefold repetition detected. Would you like a draw? [y/n]\n";
                auto draw_pair = determineIfDrawn (message);
                my_game.setProposedDrawStatus (ProposedDrawType::ThreeFoldRepetition, draw_pair);
            }
            else if (status == GameStatus::FiftyMovesWithoutProgressReached)
            {
                string message
                    = "Fifty moves without progress detected. Would you like a draw? [y/n]\n";
                auto draw_pair = determineIfDrawn (message);
                my_game.setProposedDrawStatus (ProposedDrawType::FiftyMovesWithoutProgress, draw_pair);
            }
        }

        void printAvailableMoves()
        {
            MoveList moves = generateLegalMoves (my_game.getBoard(), my_game.getCurrentTurn());
            std::cout << "\nAvailable moves:\n    ";

            int count = 0;
            for (auto move : moves)
            {
                std::cout << "[" << asString (move) << "] ";
                if (++count % 10 == 0)
                    std::cout << "\n"
                              << "    ";
            }

            std::cout << "\n\n";
        }

        static auto
        prompt (const string& prompt)
            -> string
        {
            string input;

            std::cout << prompt << "? ";

            if (!std::getline (std::cin, input))
                return "";

            return chomp (input);
        }

        static auto
        saveGame()
            -> string
        {
            string input = prompt ("save to what file");
            return input;
        }

        auto
        loadGame()
            -> optional<Game>
        {
            string input = prompt ("load what file");

            if (input.empty())
                return nullopt;

            auto optional_game = Game::loadGame (input, my_game.getPlayers());
            if (!optional_game.has_value())
                return nullopt;

            return std::move (*optional_game);
        }

        static auto
        loadFen()
            -> optional<Game>
        {
            string input = prompt ("FEN game");

            if (input.empty())
                return nullopt;

            try
            {
                Game new_game = Game::createGameFromFen (input);
                return std::move (new_game);
            }
            catch ([[maybe_unused]] FenParserError& error)
            {
                return nullopt;
            }
        }

        static auto
        readInt (const std::string& prompt_value)
            -> optional<int>
        {
            string input = prompt (prompt_value);

            if (input.empty())
                return nullopt;

            try
            {
                return std::stoi (input);
            }
            catch (std::invalid_argument& e)
            {
                return nullopt;
            }
        }

        // Copy the configuration from the old game to the new game.
        static void copyConfig (const Game& old_game, Game& new_game)
        {
            auto orig_players = old_game.getPlayers();
            auto orig_timeout = old_game.getSearchTimeout();
            auto orig_max_depth = old_game.getMaxDepth();

            new_game.setPlayers (orig_players);
            new_game.setSearchTimeout (orig_timeout);
            new_game.setMaxDepth (orig_max_depth);
        }

        static void printHelp()
        {
            std::cout
                << "\nAvailable commands:\n\n"
                << "  moves           Display available moves\n"
                << "  load            Load a game from a list of moves\n"
                << "  save            Save a game to a list of moves\n"
                << "  fen             Load a position from a FEN string\n"
                << "  pause           Pause the computer from searching for moves\n"
                << "  unpause         Unpause the computer from searching for moves\n"
                << "  maxdepth        Set the maximum depth for the computer to search\n"
                << "  timeout         Set the maximum time for the computer to search\n"
                << "  human_white     Set the white player to human\n"
                << "  human_black     Set the black player to human\n"
                << "  computer_white  Set the white player to computer\n"
                << "  computer_black  Set the black player to computer\n"
                << "  switch          Switch the current turn to the other player\n"
                << "  quit\n"
                << "  exit            Quit a game\n"
                << "  a2c3            Move the piece at a2 to c3 (example)\n"
                << "  a2xc3           Take the piece at c3 with the piece on a2 (example)\n"
                << "  o-o             Castle kingside\n"
                << "  o-o-o           Castle queenside\n"
                << "  a7a8(q)         Promote pawn to queen (example)\n"
                << "  a7xa8(q)        Capture the piece on a8 and promote the pawn to a Queen\n"
                << "  a7a8(r)         Promote pawn to rook (example)\n"
                << "  a7a8(b)         Promote pawn to bishop (example)\n"
                << "  a7a8(n)         Promote pawn to knight (example)\n"
                << "  e5f4 ep         Capture pawn with en passant if possible (example)\n"
                << "  help or ?       Display help\n"
                << "\n\n";
        }

        auto readCommand() -> PlayCommand::AnyCommand
        {
            string input;

            std::cout << "(" << wisdom::asString (my_game.getCurrentTurn()) << ")? ";

            if (!std::getline (std::cin, input))
                return PlayCommand::StopGame::fromShowFinalPosition (false);

            input = chomp (input);

            if (input == "help" || input == "?")
            {
                return PlayCommand::Help {};
            }
            else if (input == "moves")
            {
                return PlayCommand::PrintAvailableMoves {};
            }
            else if (input == "save")
            {
                auto result = saveGame();

                if (result.empty())
                    return PlayCommand::ShowError { "Missing save game filename" };

                return PlayCommand::SaveGame { result };
            }
            else if (input == "load")
            {
                auto new_game = loadGame();

                if (new_game.has_value())
                    return PlayCommand::LoadNewGame { std::move (*new_game) };
                else
                    return PlayCommand::ShowError ("Error loading game.");
            }
            else if (input == "fen")
            {
                auto new_game = loadFen();

                if (new_game.has_value())
                    return PlayCommand::LoadNewGame { std::move (*new_game) };
                else
                    return PlayCommand::ShowError { "Failed loading FEN string." };
            }
            else if (input == "pause")
            {
                return PlayCommand::Pause {};
            }
            else if (input == "unpause")
            {
                return PlayCommand::Unpause {};
            }
            else if (input == "maxdepth")
            {
                optional<int> max_depth = readInt ("Max depth");

                if (max_depth.has_value() && *max_depth >= 0)
                    return PlayCommand::SetMaxDepth { *max_depth };
                else
                    return PlayCommand::ShowError { "Invalid search depth." };
            }
            else if (input == "timeout")
            {
                optional<int> search_timeout = readInt ("Search Timeout");

                if (search_timeout.has_value() && *search_timeout >= 0)
                    return PlayCommand::SetSearchTimeout { chrono::seconds { *search_timeout } };
                else
                    return PlayCommand::ShowError { "Invalid search timeout." };
            }
            else if (input == "computer_black")
            {
                return PlayCommand::SetPlayer { Color::Black, Player::ChessEngine };
            }
            else if (input == "computer_white")
            {
                return PlayCommand::SetPlayer { Color::White, Player::ChessEngine };
            }
            else if (input == "human_white")
            {
                return PlayCommand::SetPlayer { Color::White, Player::Human };
            }
            else if (input == "human_black")
            {
                return PlayCommand::SetPlayer { Color::Black, Player::Human };
            }
            else if (input == "switch")
            {
                return PlayCommand::SwitchSides {};
            }
            else if (input == "quit" || input == "exit")
            {
                return PlayCommand::StopGame::fromShowFinalPosition (false);
            }
            else
            {
                auto optional_move = moveParseOptional (input, my_game.getCurrentTurn());
                PlayCommand::AnyCommand result = PlayCommand::ShowError { "Invalid move or command." };

                if (!optional_move.has_value())
                    return result;

                // check the generated move list for this move to see if its valid
                MoveList moves = generateLegalMoves (my_game.getBoard(), my_game.getCurrentTurn());

                for (auto legal_move : moves)
                {
                    if (optional_move.has_value() && legal_move == *optional_move)
                    {
                        result = PlayCommand::PlayMove { *optional_move };
                        break;
                    }
                }
                return result;
            }

            throw Error { "Invalid command." };
        }

        void handleCommand (const PlayCommand::AnyCommand& command)
        {
            if (holds_alternative<PlayCommand::None> (command))
            {
                // do nothing
            }
            else if (holds_alternative<PlayCommand::Help> (command))
            {
                printHelp();
            }
            else if (holds_alternative<PlayCommand::ShowError> (command))
            {
                auto error_command = get<PlayCommand::ShowError> (command);
                std::cout << "Error: \n" << error_command.message << "\n\n";
                printHelp();
            }
            else if (holds_alternative<PlayCommand::ShowInfo> (command))
            {
                auto info_command = get<PlayCommand::ShowInfo> (command);
                std::cout << "\n" << info_command.message << "\n\n";
            }
            else if (holds_alternative<PlayCommand::Pause> (command))
            {
                setPaused (true);
                std::cout << "Game engine paused.\n";
            }
            else if (holds_alternative<PlayCommand::Unpause> (command))
            {
                setPaused (false);
                std::cout << "Game engine unpaused.\n";
            }
            else if (holds_alternative<PlayCommand::StopGame> (command))
            {
                auto stop_game = get<PlayCommand::StopGame> (command);
                setQuit (true);
                setShowFinalPosition (stop_game.show_final_position);
            }
            else if (holds_alternative<PlayCommand::SaveGame> (command))
            {
                auto save_game = get<PlayCommand::SaveGame> (command);

                // todo: handle errors here
                my_game.save (save_game.file_path);
                std::cout << "Game saved to " << save_game.file_path << "\n\n";
            }
            else if (holds_alternative<PlayCommand::PrintAvailableMoves> (command))
            {
                printAvailableMoves();
            }
            else if (holds_alternative<PlayCommand::SetMaxDepth> (command))
            {
                auto max_depth_command = get<PlayCommand::SetMaxDepth> (command);
                my_game.setMaxDepth (max_depth_command.max_depth);
                std::cout << "Max depth set to " << max_depth_command.max_depth << ".\n";
            }
            else if (holds_alternative<PlayCommand::SetSearchTimeout> (command))
            {
                auto search_timeout = get<PlayCommand::SetSearchTimeout> (command);
                my_game.setSearchTimeout (chrono::seconds { search_timeout.seconds });
                std::cout << "Timeout set to " << search_timeout.seconds.count() << " seconds.\n";
            }
            else if (holds_alternative<PlayCommand::LoadNewGame> (command))
            {
                auto load_game = get<PlayCommand::LoadNewGame> (command);

                // Keep the same player config:
                copyConfig (my_game, load_game.new_game);
                my_game = std::move (load_game.new_game);
                resetStateForNewGame();

                std::cout << "\nNew game successfully loaded.\n\n";
            }
            else if (holds_alternative<PlayCommand::SwitchSides> (command))
            {
                my_game.setCurrentTurn (colorInvert (my_game.getCurrentTurn()));
                std::cout << "Players switched.\n";
            }
            else if (holds_alternative<PlayCommand::SetPlayer> (command))
            {
                auto set_player = get<PlayCommand::SetPlayer> (command);

                auto players = my_game.getPlayers();
                players[colorIndex (set_player.side)] = set_player.player_type;
                my_game.setPlayers (players);

                auto player_type_str = set_player.player_type == Player::ChessEngine
                    ? "computer"
                    : "human";
                std::cout << asString (set_player.side) << " player set to " <<
                    player_type_str << ".\n";
            }
            else if (holds_alternative<PlayCommand::PlayMove> (command))
            {
                auto play_move = get<PlayCommand::PlayMove> (command);
                my_game.move (play_move.move);
            }
            else
            {
                throw Error { "Undefined command." };
            }
        }
    };

    void ConsoleGame::play()
    {
        auto output = makeStandardLogger();

        while (true)
        {
            std::cout << my_game.getBoard() << "\n";

            // Handle draw proposals synchronously before updating displayed state:
            handleDrawProposals();
            updateDisplayedGameState();

            if (quit)
                break;

            if (!paused && my_game.getCurrentPlayer() == Player::ChessEngine)
            {
                auto optional_move = my_game.findBestMove (output);
                if (!optional_move.has_value())
                {
                    std::cout << "\nCouldn't find move!\n";
                    break;
                }

                auto move = *optional_move;
                std::cout << "move selected: [" << asString (move) << "]\n";
                my_game.move (move);
            }
            else
            {
                auto command = readCommand();
                handleCommand (command);

                if (quit && !show_final_position)
                    break;
            }
        }
    }

    void play()
    {
        ConsoleGame console_game {};
        console_game.play();
    }
}
