#include <filesystem>
#include <fstream>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/game_file.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    class TemporaryFile
    {
    public:
        explicit TemporaryFile (czstring name)
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
        czstring move_texts[] = {
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

TEST_CASE( "saveGame() writes the moves to a file without .fen in its name" )
{
    TemporaryFile file { "wisdom-chess-game-file-moves-test.txt" };
    auto game = playSampleGame();
    auto players = Players { Player::Human, Player::Human };

    SUBCASE( "It writes one move per line" )
    {
        saveGame (game, file.path());

        auto lines = file.lines();
        const auto& moves = game.getHistory().getMoveHistory();

        REQUIRE( lines.size() == moves.size() );
        for (std::size_t i = 0; i < moves.size(); i++)
            CHECK( lines[i] == asString (moves[i]) );
    }

    SUBCASE( "A saved game loads back to the same position" )
    {
        saveGame (game, file.path());

        auto loaded = loadGame (file.path(), players);

        REQUIRE( loaded.has_value() );
        CHECK( fenOf (*loaded) == fenOf (game) );
        CHECK( loaded->getHistory().getMoveHistory() == game.getHistory().getMoveHistory() );
    }

    SUBCASE( "A game without moves saves an empty file" )
    {
        auto new_game = Game::createStandardGame();

        saveGame (new_game, file.path());

        CHECK( file.lines().empty() );

        auto loaded = loadGame (file.path(), players);
        REQUIRE( loaded.has_value() );
        CHECK( fenOf (*loaded) == fenOf (new_game) );
    }
}

TEST_CASE( "saveGame() writes FEN to a .fen file" )
{
    TemporaryFile file { "wisdom-chess-game-file-fen-test.fen" };
    auto game = playSampleGame();

    saveGame (game, file.path());

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

TEST_CASE( "saveGame() reports a file it cannot write" )
{
    auto game = playSampleGame();

    CHECK_THROWS_AS( saveGame (game, "/nonexistent-directory/game.txt"), Error );
    CHECK_THROWS_AS( saveGame (game, "/nonexistent-directory/game.fen"), Error );
}

TEST_CASE( "loadGame()" )
{
    TemporaryFile file { "wisdom-chess-game-file-load-test.txt" };
    auto players = Players { Player::Human, Player::Human };

    auto write_file = [&file] (czstring contents)
    {
        std::ofstream stream { file.path() };
        stream << contents;
    };

    SUBCASE( "A missing file yields no game" )
    {
        CHECK( !loadGame (file.path(), players).has_value() );
    }

    SUBCASE( "Moves are replayed up to the stop marker" )
    {
        write_file ("e2 e4\ne7 e5\nstop\ng1 f3\n");

        auto game = loadGame (file.path(), players);

        REQUIRE( game.has_value() );
        CHECK( game->getCurrentTurn() == Color::White );
        CHECK( game->getBoard().pieceAt (coordParse ("e5")) == ColoredPiece::make (Color::Black, Piece::Pawn) );
        CHECK( game->getBoard().pieceAt (coordParse ("g1")) == ColoredPiece::make (Color::White, Piece::Knight) );
    }

    SUBCASE( "An unparseable move yields no game" )
    {
        write_file ("e2 e4\nnot a move\n");

        CHECK( !loadGame (file.path(), players).has_value() );
    }
}
