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

    auto
    UciInterface::handleGo (const vector<string>& tokens)
        -> void
    {
        auto best_move = my_game.findBestMove (my_logger);
        sendBestMove (best_move);
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