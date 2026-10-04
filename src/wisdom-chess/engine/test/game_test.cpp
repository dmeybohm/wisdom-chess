#include <algorithm>
#include <chrono>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "Initial board position is added to history" )
{
    //
    // Test that the initial board position is included in the "history", so if we
    // reach it by repetition, the draw will be detected one move sooner.
    //
    auto run_test = [] (nonnull<Game> game)
    {
        Move white_move = toMove ("g1 f3");
        Move black_move = toMove ("b8 c6");
        Move white_return_move = toMove ("f3 g1");
        Move black_return_move = toMove ("c6 b8");
        auto& history = game->getHistory();

        for (int i = 0; i < 2; i++)
        {
            INFO( i );
            game->move (white_move);
            CHECK( history.isThirdRepetition (game->getBoard()) == false );

            game->move (black_move);
            CHECK( history.isThirdRepetition (game->getBoard()) == false );

            game->move (white_return_move);
            CHECK( history.isThirdRepetition (game->getBoard()) == false );

            game->move (black_return_move);

            bool is_draw = (i == 1) ? true : false;
            CHECK( history.isThirdRepetition (game->getBoard()) == is_draw );
        }
    };

    SUBCASE( "When a default game is initialized" )
    {
        Game game = Game::createStandardGame();
        run_test (&game);
    }

    SUBCASE( "When game is created from two players" )
    {
        Game game = Game::createGame (Player::Human, Player::Human);
        run_test (&game);
    }

    SUBCASE( "When game is created from an array of two players" )
    {
        Game game = Game::createGame (Players { Player::ChessEngine, Player::ChessEngine });
        run_test (&game);
    }

    SUBCASE( "When game is created from the current turn" )
    {
        // Note: No direct factory for Color, but we can create standard and set turn
        Game game = Game::createStandardGame();
        game.setCurrentTurn (Color::White);
        run_test (&game);
    }

    SUBCASE( "When game is initialized from a board builder" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        Game game = Game::createGameFromBoard (builder);
        run_test (&game);
    }

    SUBCASE( "From FEN string" )
    {
        Game game = Game::createGameFromFen ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        run_test (&game);
    }
}

TEST_CASE( "findBestMove searches with the caller's transposition table" )
{
    auto game = Game::createStandardGame();
    game.setMaxDepth (3);
    game.setSearchTimeout (chrono::seconds { 30 });

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

TEST_CASE( "findBestMove stores the positions at the search's horizon" )
{
    auto game = Game::createStandardGame();
    game.setMaxDepth (1);
    game.setSearchTimeout (chrono::seconds { 30 });

    auto logger = makeNullLogger();
    TranspositionTable table = TranspositionTable::fromMegabytes (1);

    auto move = game.findBestMove (logger, &table);
    REQUIRE( move.has_value() );

    // The root, and the position after each of White's 20 first moves.
    CHECK( table.getStats().stored_entries == 21 );

    SUBCASE( "and answers a second search from them" )
    {
        auto stats_before = table.getStats();
        auto move_again = game.findBestMove (logger, &table);
        auto stats_after = table.getStats();

        REQUIRE( move_again.has_value() );
        CHECK( *move_again == *move );
        CHECK( stats_after.hits - stats_before.hits == 20 );
    }
}

TEST_CASE( "A draw-derived score is not reused for a position with a different clock" )
{
    //
    // The board hash covers the pieces, the side to move, castling rights and
    // en passant, but not the halfmove clock. These two positions therefore
    // share every hash in the search, while only the first is close enough to
    // the fifty-move limit for the draw rule to apply.
    //
    const auto near_fifty_move_fen = "8/8/4k3/8/8/4K3/8/7R w - - 96 200";
    const auto fresh_clock_fen = "8/8/4k3/8/8/4K3/8/7R w - - 0 200";

    auto logger = makeNullLogger();

    auto search_fen = [&] (czstring fen, nonnull<TranspositionTable> table)
    {
        auto game = Game::createGameFromFen (fen);
        game.setMaxDepth (4);
        game.setSearchTimeout (chrono::seconds { 30 });
        return game.findBestMove (logger, table);
    };

    TranspositionTable cold = TranspositionTable::fromMegabytes (1);
    auto expected = search_fen (fresh_clock_fen, &cold);
    REQUIRE( expected.has_value() );

    TranspositionTable shared = TranspositionTable::fromMegabytes (1);
    auto near_fifty_move = search_fen (near_fifty_move_fen, &shared);
    REQUIRE( near_fifty_move.has_value() );

    // The drawing scores the first search produced must not decide this one.
    auto with_warm_table = search_fen (fresh_clock_fen, &shared);

    REQUIRE( with_warm_table.has_value() );
    CHECK( *with_warm_table == *expected );
}

TEST_CASE( "findBestMove finds a move in a position that can be claimed as a draw" )
{
    auto logger = makeNullLogger();
    TranspositionTable table = TranspositionTable::fromMegabytes (1);

    SUBCASE( "Past fifty moves without progress" )
    {
        // Only the capture resets the count.
        auto game = Game::createGameFromFen ("6k1/5pp1/7p/8/8/8/8/Rn2K3 w - - 120 90");
        REQUIRE( game.getStatus() == GameStatus::FiftyMovesWithoutProgressReached );
        game.setMaxDepth (2);

        auto move = game.findBestMove (logger, &table);

        REQUIRE( move.has_value() );
        CHECK( *move == toMove ("a1xb1", Color::White) );
    }

    SUBCASE( "On the third occurrence of the position" )
    {
        auto game = Game::createGameFromFen ("6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1");
        for (int i = 0; i < 2; i++)
        {
            game.move (toMove ("e1 e2", Color::White));
            game.move (toMove ("g8 h8", Color::Black));
            game.move (toMove ("e2 e1", Color::White));
            game.move (toMove ("h8 g8", Color::Black));
        }
        REQUIRE( game.getStatus() == GameStatus::ThreefoldRepetitionReached );
        game.setMaxDepth (2);

        auto move = game.findBestMove (logger, &table);

        REQUIRE( move.has_value() );
        CHECK( *move == toMove ("a1 a8", Color::White) );
    }

    SUBCASE( "With insufficient material" )
    {
        auto game = Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
        REQUIRE( game.getStatus() == GameStatus::InsufficientMaterialDraw );
        game.setMaxDepth (2);

        auto move = game.findBestMove (logger, &table);

        CHECK( move.has_value() );
    }
}

TEST_CASE( "The draw arbiter decides the limits of the search" )
{
    const pair<DrawStatus, DrawStatus> both_declined { DrawStatus::Declined, DrawStatus::Declined };

    SUBCASE( "The game arbitrates by default, at the limits for a claim" )
    {
        auto game = Game::createStandardGame();

        CHECK( game.getDrawArbiter() == DrawArbiter::GameEngine );
        CHECK( game.getDrawLimits() == Claimable_Draw_Limits );
    }

    SUBCASE( "A declined draw raises its own limit" )
    {
        auto game = Game::createStandardGame();

        game.setProposedDrawStatus (ProposedDrawType::ThreeFoldRepetition, both_declined);
        CHECK( game.getDrawLimits().repetitions == 5 );
        CHECK( game.getDrawLimits().half_moves_without_progress == 100 );

        game.setProposedDrawStatus (ProposedDrawType::FiftyMovesWithoutProgress, both_declined);
        CHECK( game.getDrawLimits() == Automatic_Draw_Limits );
    }

    SUBCASE( "An external arbiter's limits hold whatever the players answer" )
    {
        const DrawLimits callers_limits { .repetitions = 4, .half_moves_without_progress = 120 };
        auto game = Game::createStandardGame();
        game.setExternalDrawArbiter (callers_limits);

        CHECK( game.getDrawArbiter() == DrawArbiter::External );
        CHECK( game.getDrawLimits() == callers_limits );

        game.setProposedDrawStatus (ProposedDrawType::ThreeFoldRepetition, both_declined);
        game.setProposedDrawStatus (ProposedDrawType::FiftyMovesWithoutProgress, both_declined);

        CHECK( game.getDrawLimits() == callers_limits );
    }

    SUBCASE( "A copy of the game keeps the arbiter and its limits" )
    {
        auto game = Game::createStandardGame();
        game.setExternalDrawArbiter (Automatic_Draw_Limits);

        Game copy = game;

        CHECK( copy.getDrawArbiter() == DrawArbiter::External );
        CHECK( copy.getDrawLimits() == Automatic_Draw_Limits );
    }

    SUBCASE( "findBestMove searches at the game's limits" )
    {
        // White stays well behind after taking the pawn, so it only takes
        // it when the other moves are not draws.
        auto game = Game::createGameFromFen ("1n4k1/p1pppppp/8/8/8/8/8/R3K3 w - - 120 90");
        game.setProposedDrawStatus (ProposedDrawType::FiftyMovesWithoutProgress, both_declined);
        game.setMaxDepth (2);

        auto logger = makeNullLogger();
        auto takes_pawn = toMove ("a1xa7", Color::White);

        TranspositionTable table = TranspositionTable::fromMegabytes (1);
        auto under_the_game = game.findBestMove (logger, &table);

        game.setExternalDrawArbiter (Claimable_Draw_Limits);
        table.clear();
        auto under_the_caller = game.findBestMove (logger, &table);

        REQUIRE( under_the_game.has_value() );
        CHECK( *under_the_game == takes_pawn );

        REQUIRE( under_the_caller.has_value() );
        CHECK( *under_the_caller != takes_pawn );
    }
}

TEST_CASE( "setCurrentTurn keeps the history's current position in step" )
{
    SUBCASE( "Before any move" )
    {
        auto game = Game::createStandardGame();
        game.setCurrentTurn (Color::Black);

        CHECK( game.getHistory().isCertainlyNthRepetition (game.getBoard(), 1) );
        CHECK( game.getHistory().isProbablyNthRepetition (game.getBoard(), 1) );
    }

    SUBCASE( "After a move, and the position recurs" )
    {
        auto game = Game::createStandardGame();
        game.move (toMove ("g1 f3"));
        game.setCurrentTurn (Color::White);

        CHECK( game.getHistory().isCertainlyNthRepetition (game.getBoard(), 1) );

        game.move (toMove ("f3 g1"));
        game.setCurrentTurn (Color::White);
        game.move (toMove ("g1 f3"));
        game.setCurrentTurn (Color::White);

        CHECK( game.getHistory().isCertainlyNthRepetition (game.getBoard(), 2) );
        CHECK( game.getHistory().isProbablyNthRepetition (game.getBoard(), 2) );
    }
}

TEST_CASE( "Game keeps the search depth and timeout it is given" )
{
    auto game = Game::createStandardGame();

    game.setMaxDepth (3);
    game.setSearchTimeout (chrono::milliseconds { 1 });
    CHECK( game.getMaxDepth() == 3 );
    CHECK( game.getSearchTimeout() == chrono::milliseconds { 1 } );
}
