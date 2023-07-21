#include "tests.hpp"
#include "board_builder.hpp"
#include "game.hpp"
#include "fen_parser.hpp"

using namespace wisdom;

TEST_CASE( "Initial board position is added to history" )
{
    //
    // Test that the initial board position is included in the "history", so if we
    // reach it by repetition, the draw will be detected one move sooner.
    //
    auto run_test = [](Game& game)
    {
        Move white_move = move_parse ("g1 f3");
        Move black_move = move_parse ("b8 c6");
        Move white_return_move = move_parse ("f3 g1");
        Move black_return_move = move_parse ("c6 b8");
        auto& history = game.getHistory ();

        for (int i = 0; i < 2; i++)
        {
            game.move (white_move);
            CHECK( history.is_third_repetition (game.getBoard ()) == false );

            game.move (black_move);
            CHECK( history.is_third_repetition (game.getBoard ()) == false );

            game.move (white_return_move);
            CHECK( history.is_third_repetition (game.getBoard ()) == false );

            game.move (black_return_move);
            if (i == 1)
                break;
            CHECK( history.is_third_repetition (game.getBoard ()) == false );
        }

        CHECK( history.is_third_repetition (game.getBoard ()) == true );
    };

    SUBCASE( "When a default game is initialized" )
    {
        Game game;
        run_test (game);
    }

    SUBCASE( "When game is created from two players" )
    {
        Game game { Player::Human, Player::Human };
        run_test (game);
    }

    SUBCASE( "When game is created from an array of two players" )
    {
        Game game { { Player::ChessEngine, Player::ChessEngine } };
        run_test (game);
    }

    SUBCASE( "When game is created from the current turn" )
    {
        Game game { Color::White };
        run_test (game);
    }

    SUBCASE( "When game is initialized from a board builder" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition ();
        Game game { builder };
        run_test (game);
    }

    SUBCASE( "From FEN string" )
    {
        FenParser fen_parser { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" } ;
        Game game = fen_parser.build();
        run_test (game);
    }
}
