#include "uci_interface.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/coord.hpp"

namespace wisdom
{
    UciInterface::UciInterface()
        : my_game { Game::createStandardGame() }
        , my_logger { makeNullLogger() }
    {
    }

    auto
    UciInterface::run()
        -> void
    {
        string line;
        while (std::getline (std::cin, line))
        {
            processCommand (line);
        }
    }

    auto
    UciInterface::processCommand (const string& line)
        -> void
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
        else if (command == "setoption")
        {
            handleSetOption (tokens);
        }
        else if (command == "stop")
        {
            handleStop();
        }
        else if (command == "quit")
        {
            handleQuit();
        }
        else if (command == "debug")
        {
            if (tokens.size() > 1 && tokens[1] == "on")
                my_debug_mode = true;
            else if (tokens.size() > 1 && tokens[1] == "off")
                my_debug_mode = false;
        }
    }

    auto
    UciInterface::handleUci()
        -> void
    {
        std::cout << "id name Wisdom Chess\n";
        std::cout << "id author Dave Meybohm\n";
        sendEngineInfo();
        std::cout << "uciok\n";
        std::cout.flush();
    }

    auto
    UciInterface::handleIsReady()
        -> void
    {
        std::cout << "readyok\n";
        std::cout.flush();
    }

    auto
    UciInterface::handleNewGame()
        -> void
    {
        my_game = Game::createStandardGame();
    }

    auto
    UciInterface::handlePosition (const vector<string>& tokens)
        -> void
    {
        if (tokens.size() < 2)
            return;

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

    // Backported from the UCI frontend on main: the search budget comes
    // from the clock as there, through a millisecond timer, less the
    // Move Overhead. The search itself still runs on this thread.
    auto
    UciInterface::handleGo (const vector<string>& tokens)
        -> void
    {
        optional<int> wtime, btime, winc, binc, movetime, depth;
        bool infinite = false;

        auto parse_int = [] (const string& text) -> optional<int>
        {
            try
            {
                return std::stoi (text);
            }
            catch (const std::exception&)
            {
                return nullopt;
            }
        };

        for (size_t i = 1; i < tokens.size(); i++)
        {
            const auto& token = tokens[i];
            if (token == "infinite")
            {
                infinite = true;
                continue;
            }
            if (i + 1 >= tokens.size())
                break;
            auto value = parse_int (tokens[i + 1]);
            if (token == "wtime")
                wtime = value;
            else if (token == "btime")
                btime = value;
            else if (token == "winc")
                winc = value;
            else if (token == "binc")
                binc = value;
            else if (token == "movetime")
                movetime = value;
            else if (token == "depth")
                depth = value;
            else
                continue;
            i++;
        }

        std::chrono::milliseconds search_time { 0 };
        if (movetime.has_value())
        {
            search_time = std::chrono::milliseconds {
                std::max (*movetime - my_move_overhead_ms, 1)
            };
        }
        else if (wtime.has_value() || btime.has_value())
        {
            bool white = my_game.getCurrentTurn() == Color::White;
            int time_remaining = white ? wtime.value_or (0) : btime.value_or (0);
            int increment = white ? winc.value_or (0) : binc.value_or (0);

            // Never more than half of what is left after the overhead, so a
            // short clock cannot run out. A budget of zero would mean no
            // limit, so the least is one millisecond.
            int available = std::max (time_remaining - my_move_overhead_ms, 0);
            int time_for_move = (available / 30) + increment;
            time_for_move = std::max (time_for_move, 100);
            time_for_move = std::min (time_for_move, available / 2);
            search_time = std::chrono::milliseconds { std::max (time_for_move, 1) };
        }
        else if (infinite)
        {
            search_time = std::chrono::hours { 1 };
        }

        auto saved_timeout = my_game.getSearchTimeout();
        auto saved_depth = my_game.getMaxDepth();
        if (search_time.count() > 0)
            my_game.setSearchTimeout (search_time);
        if (depth.has_value())
            my_game.setMaxDepth (std::clamp (*depth, 1, 64));

        auto best_move = my_game.findBestMove (my_logger);

        my_game.setSearchTimeout (saved_timeout);
        my_game.setMaxDepth (saved_depth);
        sendBestMove (best_move);
    }

    auto
    UciInterface::handleSetOption (const vector<string>& tokens)
        -> void
    {
        string name;
        optional<int> value;

        for (size_t i = 1; i < tokens.size(); i++)
        {
            if (tokens[i] == "name")
            {
                for (i++; i < tokens.size() && tokens[i] != "value"; i++)
                {
                    if (!name.empty())
                        name += ' ';
                    for (char c : tokens[i])
                        name += static_cast<char> (std::tolower (static_cast<unsigned char> (c)));
                }
                i--;
            }
            else if (tokens[i] == "value" && i + 1 < tokens.size())
            {
                try
                {
                    value = std::stoi (tokens[i + 1]);
                }
                catch (const std::exception&)
                {
                }
                break;
            }
        }

        if (name == "depth" && value.has_value())
            my_game.setMaxDepth (std::clamp (*value, 1, 64));
        else if (name == "move overhead" && value.has_value())
            my_move_overhead_ms = std::clamp (*value, 0, 5000);
    }

    auto
    UciInterface::handleStop()
        -> void
    {
    }

    auto
    UciInterface::handleQuit()
        -> void
    {
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

    auto
    UciInterface::sendEngineInfo()
        -> void
    {
        std::cout << "option name Depth type spin default " << Default_Max_Depth
                  << " min 1 max 64\n";
        std::cout << "option name Move Overhead type spin default 10 min 0 max 5000\n";
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

    auto
    UciInterface::sendBestMove (const optional<Move>& move)
        -> void
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
}