#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "wisdom-chess-tests.hpp"

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
