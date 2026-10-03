#include <fstream>

#include "wisdom-chess/engine/game_file.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/output_format.hpp"
#include "wisdom-chess/engine/str.hpp"

namespace wisdom
{
    void saveGame (const Game& game, const string& filename)
    {
        auto save_with = [&] (OutputFormat& format)
        {
            format.save (filename, game.getBoard(), game.getHistory(), game.getCurrentTurn());
        };

        if (filename.find (".fen") != string::npos)
        {
            FenOutputFormat format;
            save_with (format);
        }
        else
        {
            WisdomGameOutputFormat format;
            save_with (format);
        }
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
