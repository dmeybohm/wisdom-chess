#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/output_format.hpp"

#include "wisdom-chess-tests.hpp"

#include <filesystem>
#include <fstream>

using namespace wisdom;

namespace
{
    class TemporaryFile
    {
    public:
        explicit TemporaryFile (const char* name)
            : my_path { std::filesystem::temp_directory_path() / name }
        {
            std::filesystem::remove (my_path);
        }

        ~TemporaryFile()
        {
            std::error_code ignored;
            std::filesystem::remove (my_path, ignored);
        }

        TemporaryFile (const TemporaryFile&) = delete;
        auto operator= (const TemporaryFile&) -> TemporaryFile& = delete;

        [[nodiscard]] auto
        path() const
            -> string
        {
            return my_path.string();
        }

        [[nodiscard]] auto
        lines() const
            -> vector<string>
        {
            std::ifstream file { my_path };
            vector<string> result;
            string line;

            while (std::getline (file, line))
                result.push_back (line);
            return result;
        }

    private:
        std::filesystem::path my_path;
    };

    // A capture, an en passant capture and castling on both sides.
    auto
    playSampleGame()
        -> Game
    {
        auto game = Game::createGame (Player::Human, Player::Human);
        const char* move_texts[] = {
            "e2 e4", "a7 a6", "e4 e5", "d7 d5", "e5 d6 ep", "c7xd6",
            "g1 f3", "b8 c6", "f1 c4", "c8 g4", "o-o", "d8 d7",
            "d2 d3", "o-o-o",
        };

        for (auto move_text : move_texts)
            game.move (moveParse (move_text, game.getCurrentTurn()));

        return game;
    }

    auto
    fenOf (const Game& game)
        -> string
    {
        return game.getBoard().toFenString (game.getCurrentTurn());
    }
}

TEST_CASE( "WisdomGameOutputFormat" )
{
    TemporaryFile file { "wisdom-chess-output-format-moves-test.txt" };
    auto game = playSampleGame();
    auto players = Players { Player::Human, Player::Human };

    SUBCASE( "It writes one move per line" )
    {
        WisdomGameOutputFormat format;
        format.save (file.path(), game.getBoard(), game.getHistory(), game.getCurrentTurn());

        auto lines = file.lines();
        const auto& moves = game.getHistory().getMoveHistory();

        REQUIRE( lines.size() == moves.size() );
        for (std::size_t i = 0; i < moves.size(); i++)
            CHECK( lines[i] == asString (moves[i]) );
    }

    SUBCASE( "A saved game loads back to the same position" )
    {
        WisdomGameOutputFormat format;
        format.save (file.path(), game.getBoard(), game.getHistory(), game.getCurrentTurn());

        auto loaded = Game::loadGame (file.path(), players);

        REQUIRE( loaded.has_value() );
        CHECK( fenOf (*loaded) == fenOf (game) );
        CHECK( loaded->getHistory().getMoveHistory() == game.getHistory().getMoveHistory() );
    }

    SUBCASE( "A game without moves saves an empty file" )
    {
        auto new_game = Game::createStandardGame();
        WisdomGameOutputFormat format;

        format.save (
            file.path(), new_game.getBoard(), new_game.getHistory(), new_game.getCurrentTurn()
        );

        CHECK( file.lines().empty() );

        auto loaded = Game::loadGame (file.path(), players);
        REQUIRE( loaded.has_value() );
        CHECK( fenOf (*loaded) == fenOf (new_game) );
    }
}

TEST_CASE( "FenOutputFormat" )
{
    TemporaryFile file { "wisdom-chess-output-format-fen-test.txt" };
    auto game = playSampleGame();

    FenOutputFormat format;
    format.save (file.path(), game.getBoard(), game.getHistory(), game.getCurrentTurn());

    auto lines = file.lines();

    REQUIRE( lines.size() == 1 );
    CHECK( lines[0] == fenOf (game) );

    SUBCASE( "The saved position parses back to itself" )
    {
        auto loaded = Game::createGameFromFen (lines[0]);

        CHECK( fenOf (loaded) == fenOf (game) );
        CHECK( loaded.getCurrentTurn() == Color::White );
    }
}

TEST_CASE( "Game::save chooses the format from the file name" )
{
    auto game = playSampleGame();

    SUBCASE( "A .fen file holds the position" )
    {
        TemporaryFile file { "wisdom-chess-game-save-test.fen" };

        game.save (file.path());

        auto lines = file.lines();
        REQUIRE( lines.size() == 1 );
        CHECK( lines[0] == fenOf (game) );
    }

    SUBCASE( "Any other file holds the moves" )
    {
        TemporaryFile file { "wisdom-chess-game-save-test.txt" };

        game.save (file.path());

        CHECK( file.lines().size() == game.getHistory().getMoveHistory().size() );
    }
}
