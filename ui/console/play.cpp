#include "board.hpp"
#include "check.hpp"
#include "fen_parser.hpp"
#include "game.hpp"
#include "generate.hpp"
#include "global.hpp"
#include "logger.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "search.hpp"
#include "str.hpp"

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

    static auto humanWantsDraw (const string& msg) -> bool
    {
        InputState result;
        string input;

        while (toupper (input[0]) != 'Y' && toupper (input[0]) != 'N')
        {
            std::cout << msg;

            if (!std::getline (std::cin, input))
                continue;
        }

        return (input[0] == 'y' || input[1] == 'Y');
    }

    static auto playerWantsDraw (const string& msg, Player player, Color who, Game& game,
                                 bool asked_human) -> DrawStatus
    {
        if (player == Player::Human)
        {
            if (asked_human)
                return DrawStatus::Declined;

            return humanWantsDraw (msg) ? DrawStatus::Accepted : DrawStatus::Declined;
        }
        return game.computerWantsDraw (who) ? DrawStatus::Accepted : DrawStatus::Declined;
    }

    // After the third repetition, either player may request a draw.
    static auto determineIfDrawn (const string& msg, InputState input_state, Game& game)
        -> std::pair<DrawStatus, DrawStatus>
    {
        auto white_player = game.getPlayer (Color::White);

        auto white_wants_draw = playerWantsDraw (msg, white_player, Color::White, game, false);
        bool asked_human = white_player == Player::Human;
        auto black_wants_draw
            = playerWantsDraw (msg, game.getPlayer (Color::Black), Color::Black, game, asked_human);

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

        [[nodiscard]] auto getInputState() & -> InputState
        {
            return my_input_state;
        }

        void setInputState (const InputState& new_state)
        {
            my_input_state = new_state;
        }

        [[nodiscard]] auto getGame() & -> Game&
        {
            return my_game;
        }
        void getGame() && = delete;

        void handleDraw (const string& msg, ProposedDrawType draw_type)
        {
            // Recursively (one-level deep) update the status again.
            auto draw_pair = determineIfDrawn (msg, my_input_state, my_game);
            my_game.setProposedDrawStatus (draw_type, draw_pair);
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
            handleDraw (message, ProposedDrawType::ThreeFoldRepetition);
        }

        void thirdRepetitionDrawAccepted() override
        {
            std::cout
                << "Draw: threefold repetition and at least one of the players wants a draw.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void fifthRepetitionDraw() override
        {
            std::cout << "Draw: same position repeated five times.\n";
            my_input_state.command = PlayCommand::StopGame;
        }

        void fiftyMovesWithoutProgressReached() override
        {
            std::string message
                = "Fifty moves without progress detected. Would you like a draw? [y/n]\n";
            handleDraw (message, ProposedDrawType::FiftyMovesWithoutProgress);
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

    static void printAvailableMoves (Game& game, MoveGenerator& generator)
    {
        MoveList moves = generator.generateLegalMoves (game.getBoard(), game.getCurrentTurn());

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

    static string prompt (const string& prompt)
    {
        string input;
        std::cout << prompt << "? ";

        if (!std::getline (std::cin, input))
            return "";

        return chomp (input);
    }

    static void saveGame (const Game& game)
    {
        string input = prompt ("save to what file");

        if (input.empty())
            return;

        game.save (input);
    }

    static optional<Game> loadGame (const Game& current_game)
    {
        string input = prompt ("load what file");

        if (input.empty())
            return nullopt;

        return Game::load (input, current_game.getPlayers());
    }

    static optional<Game> loadFen (const Game& current_game)
    {
        string input = prompt ("FEN game");
        if (input.empty())
            return nullopt;

        try
        {
            FenParser parser { input };
            auto game = parser.build();
            game.setPlayers (current_game.getPlayers());
            return game;
        }
        catch ([[maybe_unused]] FenParserError& error)
        {
            return nullopt;
        }
    }

    static optional<int> readInt (const std::string& prompt_value)
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

    static void printHelp()
    {
        std::cout << "\nAvailable commands:\n\n"
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
                  << "\n\n";
    }

    static InputState readMove (Game& game, MoveGenerator& move_generator)
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
            printAvailableMoves (game, move_generator);
        }
        else if (input == "save")
        {
            saveGame (game);
        }
        else if (input == "load")
        {
            auto orig_players = game.getPlayers();
            auto optional_game = loadGame (game);
            if (optional_game.has_value())
            {
                game = std::move (*optional_game);
                game.setPlayers (orig_players);
            }
        }
        else if (input == "fen")
        {
            auto orig_players = game.getPlayers();
            auto optional_game = loadFen (game);
            if (optional_game.has_value())
            {
                game = std::move (*optional_game);
                game.setPlayers (orig_players);
            }
        }
        else if (input == "pause")
        {
            result.command = PlayCommand::Pause;
        }
        else if (input == "unpause")
        {
            result.command = PlayCommand::Unpause;
        }
        else if (input == "maxdepth")
        {
            optional<int> max_depth = readInt ("Max depth");
            if (max_depth.has_value())
                game.setMaxDepth (*max_depth);
        }
        else if (input == "timeout")
        {
            optional<int> search_timeout = readInt ("Search Timeout");
            if (search_timeout.has_value())
                game.setSearchTimeout (chrono::seconds { *search_timeout });
        }
        else if (input == "computer_black")
        {
            game.setBlackPlayer (Player::ChessEngine);
        }
        else if (input == "computer_white")
        {
            game.setWhitePlayer (Player::ChessEngine);
        }
        else if (input == "human_white")
        {
            game.setWhitePlayer (Player::Human);
        }
        else if (input == "human_black")
        {
            game.setBlackPlayer (Player::Human);
        }
        else if (input == "switch")
        {
            game.setCurrentTurn (colorInvert (game.getCurrentTurn()));
        }
        else if (input == "quit" || input == "exit")
        {
            result.command = PlayCommand::StopGame;
        }
        else
        {
            result.move = moveParseOptional (input, game.getCurrentTurn());
            result.command = PlayCommand::ShowError;

            if (!result.move.has_value())
            {
                printHelp();
                return result;
            }

            // check the generated move list for this move to see if its valid
            MoveList moves = move_generator.generateLegalMoves (game.getBoard(), game.getCurrentTurn());

            for (auto legal_move : moves)
            {
                if (result.move.has_value() && moveEquals (legal_move, *result.move))
                {
                    result.command = PlayCommand::PlayMove;
                    break;
                }
            }
        }

        return result;
    }

    void play()
    {
        ConsoleGameStatusManager game_status_manager {};

        InputState initial_input_state;
        auto output = makeStandardLogger();
        bool paused = false;
        MoveGenerator move_generator;

        while (true)
        {
            auto& game = game_status_manager.getGame();
            game_status_manager.setInputState (initial_input_state);

            std::cout << game.getBoard() << "\n";

            game_status_manager.update (game.status());
            auto input_state = game_status_manager.getInputState();

            if (input_state.command == PlayCommand::StopGame)
                break;

            if (!paused && game.getCurrentPlayer() == Player::ChessEngine)
            {
                auto optional_move = game.findBestMove (*output);
                if (!optional_move.has_value())
                {
                    std::cout << "\nCouldn't find move!\n";
                    break;
                }

                input_state.move = optional_move;
                if (input_state.move.has_value())
                    std::cout << "move selected: [" << asString (*input_state.move) << "]\n";
            }
            else
            {
                input_state = readMove (game, move_generator);

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

            if (input_state.move.has_value())
                game.move (*input_state.move);
        }
    }
}
