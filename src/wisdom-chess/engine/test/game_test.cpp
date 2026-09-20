#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

#include "wisdom-chess-tests.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace wisdom;

TEST_CASE( "Initial board position is added to history" )
{
    //
    // Test that the initial board position is included in the "history", so if we
    // reach it by repetition, the draw will be detected one move sooner.
    //
    auto run_test = [](Game& game)
    {
        Move white_move = moveParse ("g1 f3");
        Move black_move = moveParse ("b8 c6");
        Move white_return_move = moveParse ("f3 g1");
        Move black_return_move = moveParse ("c6 b8");
        auto& history = game.getHistory();

        for (int i = 0; i < 2; i++)
        {
            INFO( i );
            game.move (white_move);
            CHECK( history.isThirdRepetition (game.getBoard()) == false );

            game.move (black_move);
            CHECK( history.isThirdRepetition (game.getBoard()) == false );

            game.move (white_return_move);
            CHECK( history.isThirdRepetition (game.getBoard()) == false );

            game.move (black_return_move);

            bool is_draw = (i == 1) ? true : false;
            CHECK( history.isThirdRepetition (game.getBoard()) == is_draw );
        }
    };

    SUBCASE( "When a default game is initialized" )
    {
        Game game = Game::createStandardGame();
        run_test (game);
    }

    SUBCASE( "When game is created from two players" )
    {
        Game game = Game::createGame (Player::Human, Player::Human);
        run_test (game);
    }

    SUBCASE( "When game is created from an array of two players" )
    {
        Game game = Game::createGame (Players { Player::ChessEngine, Player::ChessEngine });
        run_test (game);
    }

    SUBCASE( "When game is created from the current turn" )
    {
        // Note: No direct factory for Color, but we can create standard and set turn
        Game game = Game::createStandardGame();
        game.setCurrentTurn (Color::White);
        run_test (game);
    }

    SUBCASE( "When game is initialized from a board builder" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        Game game = Game::createGameFromBoard (builder);
        run_test (game);
    }

    SUBCASE( "From FEN string" )
    {
        Game game = Game::createGameFromFen ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        run_test (game);
    }
}

TEST_CASE( "Loading a saved game" )
{
    auto path = std::filesystem::temp_directory_path() / "wisdom-chess-load-test.txt";
    auto players = Players { Player::Human, Player::Human };

    auto write_file = [&path](const char* contents)
    {
        std::ofstream file { path };
        file << contents;
    };

    SUBCASE( "A missing file yields no game" )
    {
        std::filesystem::remove (path);

        CHECK( !Game::loadGame (path.string(), players).has_value() );
    }

    SUBCASE( "Moves are replayed up to the stop marker" )
    {
        write_file ("e2 e4\ne7 e5\nstop\ng1 f3\n");

        auto game = Game::loadGame (path.string(), players);

        REQUIRE( game.has_value() );
        CHECK( game->getCurrentTurn() == Color::White );
        CHECK( game->getBoard().pieceAt (coordParse ("e5")) == ColoredPiece::make (Color::Black, Piece::Pawn) );
        CHECK( game->getBoard().pieceAt (coordParse ("g1")) == ColoredPiece::make (Color::White, Piece::Knight) );
    }

    SUBCASE( "An unparseable move yields no game" )
    {
        write_file ("e2 e4\nnot a move\n");

        CHECK( !Game::loadGame (path.string(), players).has_value() );
    }

    std::filesystem::remove (path);
}

TEST_CASE( "findBestMove searches with the caller's transposition table" )
{
    auto game = Game::createStandardGame();
    game.setMaxDepth (3);
    game.setSearchTimeout (std::chrono::seconds { 30 });

    auto logger = makeNullLogger();
    TranspositionTable table = TranspositionTable::fromMegabytes (1);

    REQUIRE( table.getStats().stored_entries == 0 );

    auto first_move = game.findBestMove (logger, &table);

    REQUIRE( first_move.has_value() );
    CHECK( table.getStats().stored_entries > 0 );

    SUBCASE( "the warm table is reused by the next search" )
    {
        game.move (*first_move);

        auto stats_before = table.getStats();
        auto second_move = game.findBestMove (logger, &table);
        auto stats_after = table.getStats();

        REQUIRE( second_move.has_value() );

        MoveList legal_moves = generateLegalMoves (game.getBoard(), game.getCurrentTurn());
        CHECK( std::find (legal_moves.begin(), legal_moves.end(), *second_move)
               != legal_moves.end() );

        // Entries written under the first root are probed under the second.
        CHECK( stats_after.hits > stats_before.hits );
    }

    SUBCASE( "a cleared table starts cold again" )
    {
        table.clear();
        CHECK( table.getStats().stored_entries == 0 );
        CHECK( table.getStats().hits == 0 );

        auto move_again = game.findBestMove (logger, &table);

        REQUIRE( move_again.has_value() );
        CHECK( *move_again == *first_move );
    }
}
