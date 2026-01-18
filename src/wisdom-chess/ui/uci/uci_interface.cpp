#include "uci_interface.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/coord.hpp"

#include <algorithm>
#include <chrono>

namespace wisdom
{
    namespace
    {
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
                {
                    std::cout << "info string " << output << "\n";
                    std::cout.flush();
                }
            }

            void info (const string& output) const override
            {
                std::cout << "info " << output << "\n";
                std::cout.flush();
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
    }

    UciInterface::UciInterface()
        : my_game { Game::createStandardGame() }
        , my_logger { makeNullLogger() }
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
            my_search_id.fetch_add (1);
            my_search_thread.join();
        }
    }

    void UciInterface::run()
    {
        string line;
        while (std::getline (std::cin, line))
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
        std::cout << "id name Wisdom Chess\n";
        std::cout << "id author Dave Meybohm\n";
        sendEngineInfo();
        std::cout << "uciok\n";
        std::cout.flush();
    }

    void UciInterface::handleIsReady()
    {
        waitForSearchThread();
        std::cout << "readyok\n";
        std::cout.flush();
    }

    void UciInterface::handleNewGame()
    {
        waitForSearchThread();
        std::lock_guard<std::mutex> lock { my_game_mutex };
        my_game = Game::createStandardGame();
    }

    void UciInterface::handlePosition (const vector<string>& tokens)
    {
        if (tokens.size() < 2)
            return;

        waitForSearchThread();
        std::lock_guard<std::mutex> lock { my_game_mutex };

        if (tokens[1] == "startpos")
        {
            my_game = Game::createStandardGame();

            auto moves_it = std::find (tokens.begin(), tokens.end(), "moves");
            if (moves_it != tokens.end())
            {
                for (auto it = moves_it + 1; it != tokens.end(); ++it)
                {
                    auto move = parseUciMove (*it);
                    if (move)
                    {
                        my_game.move (*move);
                    }
                }
            }
        }
        else if (tokens[1] == "fen" && tokens.size() >= 8)
        {
            string fen_string;
            for (size_t i = 2; i < 8 && i < tokens.size(); ++i)
            {
                if (i > 2)
                    fen_string += " ";
                fen_string += tokens[i];
            }

            try
            {
                my_game = Game::createGameFromFen (fen_string);
            }
            catch (...)
            {
                if (my_debug_mode)
                {
                    std::cout << "info string Invalid FEN: " << fen_string << "\n";
                    std::cout.flush();
                }
                return;
            }

            auto moves_it = std::find (tokens.begin(), tokens.end(), "moves");
            if (moves_it != tokens.end())
            {
                for (auto it = moves_it + 1; it != tokens.end(); ++it)
                {
                    auto move = parseUciMove (*it);
                    if (move)
                    {
                        my_game.move (*move);
                    }
                }
            }
        }
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

            int time_for_move = (time_remaining / 30) + increment;
            time_for_move = std::max (time_for_move, 100);
            search_time = std::chrono::milliseconds { time_for_move };
        }
        else if (infinite)
        {
            search_time = std::chrono::hours { 24 };
        }

        int current_search_id = my_search_id.fetch_add (1) + 1;

        Game game_copy = [this]
        {
            std::lock_guard<std::mutex> lock { my_game_mutex };
            return my_game;
        }();

        my_search_thread = std::jthread (
            [this, game = std::move (game_copy), search_depth, search_time, current_search_id] () mutable
            {
                game.setMaxDepth (search_depth);
                if (search_time.count() > 0)
                {
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds> (search_time);
                    if (seconds.count() == 0)
                        seconds = std::chrono::seconds { 1 };
                    game.setSearchTimeout (seconds);
                }
                game.setPeriodicFunction (buildNotifier (current_search_id));

                auto logger = std::make_shared<UciLogger> (my_debug_mode);
                auto best_move = game.findBestMove (logger);

                if (my_search_id.load() == current_search_id)
                {
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
        }
        else if (option_name == "depth" && value.has_value())
        {
            my_settings.default_depth = std::clamp (*value, 1, 64);
        }
    }

    void UciInterface::handleStop()
    {
        my_search_id.fetch_add (1);
    }

    void UciInterface::handleQuit()
    {
        waitForSearchThread();
        std::exit (0);
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
        std::cout << "option name Hash type spin default 16 min 1 max 1024\n";
        std::cout << "option name Depth type spin default " << Default_Max_Depth
                  << " min 1 max 64\n";
    }

    auto
    UciInterface::parseUciMove (const string& uci_move)
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
                switch (tolower (promotion_char))
                {
                    case 'q': promoted_piece = Piece::Queen; break;
                    case 'r': promoted_piece = Piece::Rook; break;
                    case 'b': promoted_piece = Piece::Bishop; break;
                    case 'n': promoted_piece = Piece::Knight; break;
                    default: return nullopt;
                }
            }

            return mapCoordinatesToMove (
                my_game.getBoard(),
                my_game.getCurrentTurn(),
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
            ColoredPiece promoted = move.getPromotedPiece();
            char piece_char = tolower (pieceToChar (promoted));
            result += piece_char;
        }

        return result;
    }

    void UciInterface::sendBestMove (const optional<Move>& move)
    {
        if (move)
        {
            std::cout << "bestmove " << moveToUci (*move) << "\n";
        }
        else
        {
            std::cout << "bestmove (none)\n";
        }
        std::cout.flush();
    }

    auto
    UciInterface::buildNotifier (int initial_search_id)
        -> MoveTimer::PeriodicFunction
    {
        return [this, initial_search_id] (not_null<MoveTimer*> timer)
        {
            if (my_search_id.load() != initial_search_id)
            {
                timer->setTriggered (true);
                timer->setCancelled (true);
            }
        };
    }
}
