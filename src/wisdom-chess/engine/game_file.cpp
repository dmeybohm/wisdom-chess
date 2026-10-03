#include <fstream>

#include "wisdom-chess/engine/game_file.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/str.hpp"

namespace wisdom
{
    namespace
    {
        auto
        openForWriting (const string& filename)
            -> std::ofstream
        {
            std::ofstream file { filename };
            if (!file)
                throw Error { "Cannot open " + filename + " for writing." };
            return file;
        }

        void
        closeAfterWriting (std::ofstream& file, const string& filename)
        {
            file.close();
            if (!file)
                throw Error { "Error writing " + filename + "." };
        }

        void
        saveFen (const Game& game, const string& filename)
        {
            auto file = openForWriting (filename);
            file << game.getBoard().toFenString (game.getCurrentTurn()) << "\n";
            closeAfterWriting (file, filename);
        }

        // One move per line, as loadGame() reads them.
        void
        saveMoveList (const Game& game, const string& filename)
        {
            auto file = openForWriting (filename);
            for (auto move : game.getHistory().getMoveHistory())
                file << asString (move) << "\n";
            closeAfterWriting (file, filename);
        }
    }

    void saveGame (const Game& game, const string& filename)
    {
        if (filename.find (".fen") != string::npos)
            saveFen (game, filename);
        else
            saveMoveList (game, filename);
    }

    auto
    loadGame (const string& filename, const Players& players)
        -> optional<Game>
    {
        string input_buf;
        std::ifstream istream;

        istream.open (filename, std::ios::in);

        if (istream.fail())
            return {};

        Game result = Game::createGame (players);

        while (std::getline (istream, input_buf))
        {
            input_buf = chomp (input_buf);

            if (input_buf == "stop")
                break;

            auto move = moveParseOptional (input_buf, result.getCurrentTurn());
            if (!move.has_value())
                return {};

            result.move (*move);
        }

        return result;
    }
}
