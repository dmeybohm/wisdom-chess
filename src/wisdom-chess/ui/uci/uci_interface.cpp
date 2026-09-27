#include "uci_interface.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/coord.hpp"
#include "wisdom-chess/engine/generate.hpp"

#include <algorithm>
#include <chrono>
#include <random>

namespace wisdom
{
    namespace
    {
        std::timed_mutex output_mutex;

        // The search thread and the command loop both write to stdout, so
        // each line is written whole under a lock.
        void sendLine (const string& line)
        {
            std::lock_guard<std::timed_mutex> lock { output_mutex };
            std::cout << line << "\n";
            std::cout.flush();
        }

        // How long a fatal message waits for the output lock. The process is
        // about to abort, so it must not hang behind a writer that is blocked.
        constexpr auto Emergency_Output_Lock_Wait = std::chrono::milliseconds { 250 };

        // Writes every line of the message as its own "info string", under the
        // output lock when it can be had in time, and without allocating.
        void sendEmergencyLines (string_view message)
        {
            std::unique_lock<std::timed_mutex> lock { output_mutex, std::defer_lock };
            [[maybe_unused]] bool locked = lock.try_lock_for (Emergency_Output_Lock_Wait);

            if (!message.empty() && message.back() == '\n')
                message.remove_suffix (1);

            while (true)
            {
                auto line_end = message.find ('\n');
                std::cout << "info string " << message.substr (0, line_end) << '\n';

                if (line_end == string_view::npos)
                    break;
                message.remove_prefix (line_end + 1);
            }
            std::cout.flush();
        }

        // For a search that ended before completing any depth.
        auto
        pickRandomLegalMove (const Game& game)
            -> optional<Move>
        {
            auto moves = generateLegalMoves (game.getBoard(), game.getCurrentTurn());
            if (moves.isEmpty())
                return nullopt;

            std::random_device random_device;
            std::mt19937 rng { random_device() };
            std::uniform_int_distribution<std::size_t> pick { 0, moves.size() - 1 };

            return *(moves.begin() + narrow<std::ptrdiff_t> (pick (rng)));
        }

        class UciLogger : public Logger
        {
        public:
            explicit UciLogger (bool debug_enabled)
                : my_debug_enabled { debug_enabled }
            {
            }

            void debug (const string& output) const override
            {
                if (my_debug_enabled)
                    sendLine ("info string " + output);
            }

            void info (const string& output) const override
            {
                sendLine ("info " + output);
            }

            void emergency (const string& output) const override
            {
                sendEmergencyLines (output);
            }

        private:
            bool my_debug_enabled;
        };

        auto
        findTokenValue (const vector<string>& tokens, const string& name)
            -> optional<int>
        {
            auto it = std::find (tokens.begin(), tokens.end(), name);
            if (it != tokens.end() && (it + 1) != tokens.end())
            {
                try
                {
                    return std::stoi (*(it + 1));
                }
                catch (...)
                {
                    return nullopt;
                }
            }
            return nullopt;
        }

        auto
        hasToken (const vector<string>& tokens, const string& name)
            -> bool
        {
            return std::find (tokens.begin(), tokens.end(), name) != tokens.end();
        }

        auto
        toLower (string str)
            -> string
        {
            std::transform (str.begin(), str.end(), str.begin(),
                [] (unsigned char c) { return std::tolower (c); });
            return str;
        }

        // A "position" command continues the current game when it starts from
        // the same place and its move list begins with the moves already
        // played. Anything else is a different game or an unrelated analysis
        // position, whose search must not see scores that a previous history
        // produced: a repetition draw is a property of the path, not of the
        // board, but the table is keyed by the board alone.
        [[nodiscard]] auto
        continuesPosition (const vector<string>& previous, const vector<string>& current)
            -> bool
        {
            auto previous_moves = std::find (previous.begin(), previous.end(), "moves");
            auto current_moves = std::find (current.begin(), current.end(), "moves");

            // The tokens before "moves" name the starting position.
            if (!std::equal (
                    previous.begin(), previous_moves,
                    current.begin(), current_moves
                ))
            {
                return false;
            }

            // The moves already played must be a prefix of the new move list.
            if (std::distance (previous_moves, previous.end())
                > std::distance (current_moves, current.end()))
            {
                return false;
            }

            return std::equal (previous_moves, previous.end(), current_moves);
        }
    }

    UciInterface::UciInterface()
        : my_game { Game::createStandardGame() }
        , my_logger { makeNullLogger() }
        , my_transposition_table { TranspositionTable::fromMegabytes (my_settings.hash_size_mb) }
    {
    }

    UciInterface::~UciInterface()
    {
        waitForSearchThread();
    }

    void UciInterface::waitForSearchThread()
    {
        if (my_search_thread.joinable())
        {
            // A stopped search still owes the GUI its bestmove, so let it finish.
            if (!my_stop_requested.load())
                my_search_id.fetch_add (1);
            my_search_thread.join();
        }
    }

    void UciInterface::run()
    {
        string line;
        while (!my_quit_requested && std::getline (std::cin, line))
        {
            processCommand (line);
        }
    }

    void UciInterface::processCommand (const string& line)
    {
        auto tokens = tokenizeCommand (line);
        if (tokens.empty())
            return;

        const auto& command = tokens[0];

        if (command == "uci")
        {
            handleUci();
        }
        else if (command == "isready")
        {
            handleIsReady();
        }
        else if (command == "ucinewgame")
        {
            handleNewGame();
        }
        else if (command == "position")
        {
            handlePosition (tokens);
        }
        else if (command == "go")
        {
            handleGo (tokens);
        }
        else if (command == "stop")
        {
            handleStop();
        }
        else if (command == "quit")
        {
            handleQuit();
        }
        else if (command == "setoption")
        {
            handleSetOption (tokens);
        }
        else if (command == "debug")
        {
            if (tokens.size() > 1 && tokens[1] == "on")
                my_debug_mode = true;
            else if (tokens.size() > 1 && tokens[1] == "off")
                my_debug_mode = false;
        }
    }

    void UciInterface::handleUci()
    {
        sendLine ("id name Wisdom Chess");
        sendLine ("id author Dave Meybohm");
        sendEngineInfo();
        sendLine ("uciok");
    }

    void UciInterface::handleIsReady()
    {
        sendLine ("readyok");
    }

    void UciInterface::handleNewGame()
    {
        waitForSearchThread();
        my_transposition_table.clear();
        my_position_tokens.clear();
        std::lock_guard<std::mutex> lock { my_game_mutex };
        my_game = Game::createStandardGame();
    }

    void UciInterface::handlePosition (const vector<string>& tokens)
    {
        if (tokens.size() < 2)
            return;

        waitForSearchThread();
        std::lock_guard<std::mutex> lock { my_game_mutex };

        Game new_game = Game::createStandardGame();
        auto moves_it = std::find (tokens.begin(), tokens.end(), "moves");

        if (tokens[1] == "startpos")
        {
            // Already the standard game.
        }
        else if (tokens[1] == "fen")
        {
            // The FEN's fields run up to "moves" or the end of the line. The
            // two clocks may be left off, as some GUIs do.
            vector<string> fields { tokens.begin() + 2, moves_it };
            if (fields.size() < 4 || fields.size() > 6)
            {
                sendLine ("info string Invalid FEN: expected 4 to 6 fields");
                return;
            }
            if (fields.size() == 4)
                fields.emplace_back ("0");
            if (fields.size() == 5)
                fields.emplace_back ("1");

            string fen_string = join (fields, " ");
            try
            {
                new_game = Game::createGameFromFen (fen_string);
            }
            catch (const Error& e)
            {
                sendLine ("info string Invalid FEN: " + fen_string + " (" + e.message() + ")");
                return;
            }
        }
        else
        {
            return;
        }

        if (moves_it != tokens.end() && !applyMoves (new_game, moves_it + 1, tokens.end()))
            return;

        my_game = std::move (new_game);

        if (!continuesPosition (my_position_tokens, tokens))
            my_transposition_table.clear();

        my_position_tokens = tokens;
    }

    void UciInterface::handleGo (const vector<string>& tokens)
    {
        waitForSearchThread();

        auto depth = findTokenValue (tokens, "depth");
        auto movetime = findTokenValue (tokens, "movetime");
        auto wtime = findTokenValue (tokens, "wtime");
        auto btime = findTokenValue (tokens, "btime");
        auto winc = findTokenValue (tokens, "winc");
        auto binc = findTokenValue (tokens, "binc");
        bool infinite = hasToken (tokens, "infinite");

        int search_depth = my_settings.default_depth;
        std::chrono::milliseconds search_time { 0 };

        if (depth.has_value())
        {
            search_depth = std::clamp (*depth, 1, 64);
        }

        if (movetime.has_value())
        {
            search_time = std::chrono::milliseconds { *movetime };
        }
        else if (wtime.has_value() || btime.has_value())
        {
            std::lock_guard<std::mutex> lock { my_game_mutex };
            Color current_turn = my_game.getCurrentTurn();

            int time_remaining = 0;
            int increment = 0;

            if (current_turn == Color::White)
            {
                time_remaining = wtime.value_or (0);
                increment = winc.value_or (0);
            }
            else
            {
                time_remaining = btime.value_or (0);
                increment = binc.value_or (0);
            }

            // Never more than half of what is left after the overhead, so a
            // short clock cannot run out. A budget of zero would mean no
            // limit, so the least is one millisecond.
            int available = std::max (time_remaining - my_settings.move_overhead_ms, 0);
            int time_for_move = (available / 30) + increment;
            time_for_move = std::max (time_for_move, 100);
            time_for_move = std::min (time_for_move, available / 2);
            search_time = std::chrono::milliseconds { std::max (time_for_move, 1) };
        }
        else if (infinite)
        {
            search_time = std::chrono::hours { 24 };
        }

        int current_search_id = my_search_id.fetch_add (1) + 1;
        my_stop_requested.store (false);

        Game game_copy = [this]
        {
            std::lock_guard<std::mutex> lock { my_game_mutex };
            return my_game;
        }();

        // The game is copied per search so that a later "position" cannot
        // disturb it, but the table is lent to the thread so that what one
        // search learns is available to the next.
        nonnull_observer_ptr<TranspositionTable> table = &my_transposition_table;

        my_search_thread = std::thread (
            [this, game = std::move (game_copy), table, search_depth, search_time,
             current_search_id, debug_mode = my_debug_mode] () mutable
            {
                game.setMaxDepth (search_depth);
                if (search_time.count() > 0)
                    game.setSearchTimeout (search_time);
                game.setPeriodicFunction (buildNotifier (current_search_id));

                auto logger = makeUciLogger (debug_mode);
                auto best_move = game.findBestMove (logger, table);

                if (my_search_id.load() == current_search_id)
                {
                    if (!best_move.has_value())
                        best_move = pickRandomLegalMove (game);

                    sendBestMove (best_move);
                }
            });
    }

    void UciInterface::handleSetOption (const vector<string>& tokens)
    {
        auto name_it = std::find (tokens.begin(), tokens.end(), "name");
        auto value_it = std::find (tokens.begin(), tokens.end(), "value");

        if (name_it == tokens.end() || name_it + 1 == tokens.end())
            return;

        string option_name;
        auto it = name_it + 1;
        while (it != tokens.end() && it != value_it)
        {
            if (!option_name.empty())
                option_name += " ";
            option_name += *it;
            ++it;
        }

        option_name = toLower (option_name);

        optional<int> value;
        if (value_it != tokens.end() && value_it + 1 != tokens.end())
        {
            try
            {
                value = std::stoi (*(value_it + 1));
            }
            catch (...)
            {
            }
        }

        if (option_name == "hash" && value.has_value())
        {
            my_settings.hash_size_mb = std::clamp (*value, 1, 1024);

            // The running search holds the table, so it has to finish first.
            waitForSearchThread();
            my_transposition_table = TranspositionTable::fromMegabytes (my_settings.hash_size_mb);
        }
        else if (option_name == "depth" && value.has_value())
        {
            my_settings.default_depth = std::clamp (*value, 1, 64);
        }
        else if (option_name == "move overhead" && value.has_value())
        {
            my_settings.move_overhead_ms =
                std::clamp (*value, 0, UciSettings::Max_Move_Overhead_Ms);
        }
    }

    void UciInterface::handleStop()
    {
        my_stop_requested.store (true);
    }

    void UciInterface::handleQuit()
    {
        waitForSearchThread();
        my_quit_requested = true;
    }

    auto
    UciInterface::tokenizeCommand (const string& line)
        -> vector<string>
    {
        std::istringstream iss (line);
        vector<string> tokens;
        string token;

        while (iss >> token)
        {
            tokens.push_back (token);
        }

        return tokens;
    }

    void UciInterface::sendEngineInfo()
    {
        sendLine ("option name Hash type spin default 16 min 1 max 1024");
        sendLine ("option name Depth type spin default " + std::to_string (Default_Max_Depth)
                  + " min 1 max 64");
        sendLine ("option name Move Overhead type spin default "
                  + std::to_string (UciSettings::Default_Move_Overhead_Ms)
                  + " min 0 max " + std::to_string (UciSettings::Max_Move_Overhead_Ms));
    }

    auto
    UciInterface::applyMoves (
        Game& game,
        vector<string>::const_iterator first,
        vector<string>::const_iterator last
    ) -> bool
    {
        for (auto it = first; it != last; ++it)
        {
            auto move = parseUciMove (game, *it);
            if (!move.has_value())
            {
                sendLine ("info string Unparseable move in position: " + *it);
                return false;
            }

            auto legal_moves = generateLegalMoves (game.getBoard(), game.getCurrentTurn());
            if (std::find (legal_moves.begin(), legal_moves.end(), *move) == legal_moves.end())
            {
                sendLine ("info string Illegal move in position: " + *it);
                return false;
            }

            game.move (*move);
        }

        return true;
    }

    auto
    UciInterface::parseUciMove (const Game& game, const string& uci_move)
        -> optional<Move>
    {
        if (uci_move.length() < 4)
            return nullopt;

        try
        {
            auto src_coord = coordParse (uci_move.substr (0, 2));
            auto dst_coord = coordParse (uci_move.substr (2, 2));

            optional<Piece> promoted_piece = nullopt;
            if (uci_move.length() == 5)
            {
                char promotion_char = uci_move[4];
                switch (wisdom::toLower (promotion_char))
                {
                    case 'q': promoted_piece = Piece::Queen; break;
                    case 'r': promoted_piece = Piece::Rook; break;
                    case 'b': promoted_piece = Piece::Bishop; break;
                    case 'n': promoted_piece = Piece::Knight; break;
                    default: return nullopt;
                }
            }

            return mapCoordinatesToMove (
                game.getBoard(),
                game.getCurrentTurn(),
                src_coord,
                dst_coord,
                promoted_piece
            );
        }
        catch (...)
        {
            return nullopt;
        }
    }

    auto
    UciInterface::moveToUci (const Move& move)
        -> string
    {
        string result;
        result += asString (move.getSrc());
        result += asString (move.getDst());

        if (move.isPromoting())
        {
            char piece_char = wisdom::toLower (pieceToChar (move.getPromotedPiece()));
            result += piece_char;
        }

        return result;
    }

    void UciInterface::sendBestMove (const optional<Move>& move)
    {
        sendLine ("bestmove " + (move ? moveToUci (*move) : string { "0000" }));
    }

    auto
    UciInterface::buildNotifier (int initial_search_id)
        -> MoveTimer::PeriodicFunction
    {
        return [this, initial_search_id] (nonnull_observer_ptr<MoveTimer> timer)
        {
            if (my_search_id.load() != initial_search_id)
            {
                timer->setCancelled (true);
            }
            else if (my_stop_requested.load())
            {
                // "stop" means the time is up: the search ends through its
                // normal timeout and keeps the last completed depth.
                timer->setTimeLimit (chrono::milliseconds { 0 });
            }
        };
    }

    auto
    makeUciLogger (bool debug_enabled)
        -> std::shared_ptr<Logger>
    {
        return std::make_shared<UciLogger> (debug_enabled);
    }
}
