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
        writeLines (const string& filename, const vector<string>& lines)
            -> expected<void, string>
        {
            std::ofstream file { filename };
            if (!file)
                return unexpected<string> { "Cannot open " + filename + " for writing." };

            for (const auto& line : lines)
                file << line << "\n";

            file.close();
            if (!file)
                return unexpected<string> { "Error writing " + filename + "." };

            return {};
        }
    }

    auto
    saveGame (const Game& game, const string& filename)
        -> expected<void, string>
    {
        if (filename.find (".fen") != string::npos)
            return writeLines (filename, { game.getBoard().toFenString (game.getCurrentTurn()) });

        // One move per line, as loadGame() reads them.
        vector<string> moves;
        for (auto move : game.getHistory().getMoveHistory())
            moves.push_back (asString (move));
        return writeLines (filename, moves);
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

            auto move = parseMove (input_buf, result.getCurrentTurn());
            if (!move.has_value())
                return {};

            result.move (*move);
        }

        return result;
    }
}
