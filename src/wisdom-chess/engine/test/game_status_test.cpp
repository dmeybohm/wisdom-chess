#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/game_status.hpp"
#include "wisdom-chess/engine/history.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    // Overrides only the two hooks, so it sees what the default
    // fine-grained methods pass on.
    class HookRecorder : public GameStatusUpdate
    {
    public:
        vector<GameStatus> ended;
        vector<ProposedDrawType> proposed;

    protected:
        void onGameEnded (GameStatus status) override
        {
            ended.push_back (status);
        }

        void onDrawProposed (ProposedDrawType type) override
        {
            proposed.push_back (type);
        }
    };

    // Overrides every fine-grained method, so the hooks must stay silent.
    class MethodRecorder : public GameStatusUpdate
    {
    public:
        vector<string> calls;
        int hook_calls = 0;

        void checkmate() override { calls.emplace_back ("checkmate"); }
        void stalemate() override { calls.emplace_back ("stalemate"); }
        void insufficientMaterial() override { calls.emplace_back ("insufficientMaterial"); }
        void thirdRepetitionDrawReached() override { calls.emplace_back ("thirdReached"); }
        void thirdRepetitionDrawAccepted() override { calls.emplace_back ("thirdAccepted"); }
        void fifthRepetitionDraw() override { calls.emplace_back ("fifth"); }
        void fiftyMovesWithoutProgressReached() override { calls.emplace_back ("fiftyReached"); }
        void fiftyMovesWithoutProgressAccepted() override { calls.emplace_back ("fiftyAccepted"); }
        void seventyFiveMovesWithNoProgress() override { calls.emplace_back ("seventyFive"); }

    protected:
        void onGameEnded ([[maybe_unused]] GameStatus status) override
        {
            hook_calls++;
        }

        void onDrawProposed ([[maybe_unused]] ProposedDrawType type) override
        {
            hook_calls++;
        }
    };

    void shuffleKnights (Game& game, int times)
    {
        for (int i = 0; i < times; i++)
        {
            game.move (moveParse ("g1 f3", Color::White));
            game.move (moveParse ("g8 f6", Color::Black));
            game.move (moveParse ("f3 g1", Color::White));
            game.move (moveParse ("f6 g8", Color::Black));
        }
    }
}

TEST_CASE( "GameStatusUpdate dispatches to the hooks by default" )
{
    HookRecorder recorder;

    SUBCASE( "Playing calls nothing" )
    {
        recorder.update (GameStatus::Playing);

        CHECK( recorder.ended.empty() );
        CHECK( recorder.proposed.empty() );
    }

    SUBCASE( "Each game-ending status reaches onGameEnded() unchanged" )
    {
        auto ending_statuses = {
            GameStatus::Checkmate,
            GameStatus::Stalemate,
            GameStatus::InsufficientMaterialDraw,
            GameStatus::ThreefoldRepetitionAccepted,
            GameStatus::FivefoldRepetitionDraw,
            GameStatus::FiftyMovesWithoutProgressAccepted,
            GameStatus::SeventyFiveMovesWithoutProgressDraw,
        };

        for (auto status : ending_statuses)
            recorder.update (status);

        CHECK( recorder.ended == vector<GameStatus> { ending_statuses } );
        CHECK( recorder.proposed.empty() );
    }

    SUBCASE( "Each reached draw becomes a proposal" )
    {
        recorder.update (GameStatus::ThreefoldRepetitionReached);
        recorder.update (GameStatus::FiftyMovesWithoutProgressReached);

        CHECK( recorder.ended.empty() );
        CHECK( recorder.proposed == vector<ProposedDrawType> {
            ProposedDrawType::ThreeFoldRepetition,
            ProposedDrawType::FiftyMovesWithoutProgress,
        } );
    }
}

TEST_CASE( "GameStatusUpdate calls the fine-grained method for each status" )
{
    MethodRecorder recorder;

    recorder.update (GameStatus::Playing);
    recorder.update (GameStatus::Checkmate);
    recorder.update (GameStatus::Stalemate);
    recorder.update (GameStatus::ThreefoldRepetitionReached);
    recorder.update (GameStatus::ThreefoldRepetitionAccepted);
    recorder.update (GameStatus::FivefoldRepetitionDraw);
    recorder.update (GameStatus::FiftyMovesWithoutProgressReached);
    recorder.update (GameStatus::FiftyMovesWithoutProgressAccepted);
    recorder.update (GameStatus::SeventyFiveMovesWithoutProgressDraw);
    recorder.update (GameStatus::InsufficientMaterialDraw);

    CHECK( recorder.calls == vector<string> {
        "checkmate",
        "stalemate",
        "thirdReached",
        "thirdAccepted",
        "fifth",
        "fiftyReached",
        "fiftyAccepted",
        "seventyFive",
        "insufficientMaterial",
    } );
    CHECK( recorder.hook_calls == 0 );
}

TEST_CASE( "Game::status" )
{
    SUBCASE( "A new game is being played" )
    {
        auto game = Game::createStandardGame();

        CHECK( game.status() == GameStatus::Playing );
    }

    SUBCASE( "Checkmate, reached by playing the moves" )
    {
        auto game = Game::createStandardGame();

        game.move (moveParse ("f2 f3", Color::White));
        game.move (moveParse ("e7 e5", Color::Black));
        game.move (moveParse ("g2 g4", Color::White));
        CHECK( game.status() == GameStatus::Playing );

        game.move (moveParse ("d8 h4", Color::Black));
        CHECK( game.status() == GameStatus::Checkmate );
    }

    SUBCASE( "Checkmate of Black" )
    {
        auto game = Game::createGameFromFen (
            "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4"
        );

        CHECK( game.status() == GameStatus::Checkmate );
    }

    SUBCASE( "Stalemate of the player to move" )
    {
        auto black_to_move = Game::createGameFromFen ("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
        auto white_to_move = Game::createGameFromFen ("7K/5q2/6k1/8/8/8/8/8 w - - 0 1");

        CHECK( black_to_move.status() == GameStatus::Stalemate );
        CHECK( white_to_move.status() == GameStatus::Stalemate );
    }

    SUBCASE( "A check is still playing" )
    {
        auto game = Game::createGameFromFen ("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");

        CHECK( game.status() == GameStatus::Playing );
    }

    SUBCASE( "Insufficient material" )
    {
        auto bare_kings = Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
        auto lone_knight = Game::createGameFromFen ("4k3/8/8/8/8/8/8/4KN2 w - - 0 1");
        auto lone_pawn = Game::createGameFromFen ("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");

        CHECK( bare_kings.status() == GameStatus::InsufficientMaterialDraw );
        CHECK( lone_knight.status() == GameStatus::InsufficientMaterialDraw );
        CHECK( lone_pawn.status() == GameStatus::Playing );
    }

    SUBCASE( "Threefold repetition" )
    {
        auto game = Game::createStandardGame();

        shuffleKnights (game, 1);
        CHECK( game.status() == GameStatus::Playing );

        shuffleKnights (game, 1);
        CHECK( game.status() == GameStatus::ThreefoldRepetitionReached );

        SUBCASE( "Accepted by both players" )
        {
            game.setProposedDrawStatus (
                ProposedDrawType::ThreeFoldRepetition,
                { DrawStatus::Accepted, DrawStatus::Accepted }
            );

            CHECK( game.status() == GameStatus::ThreefoldRepetitionAccepted );
        }

        SUBCASE( "One player claiming the draw is enough" )
        {
            game.setProposedDrawStatus (
                ProposedDrawType::ThreeFoldRepetition,
                { DrawStatus::Declined, DrawStatus::Accepted }
            );

            CHECK( game.status() == GameStatus::ThreefoldRepetitionAccepted );
        }

        SUBCASE( "Nothing changes until both players have replied" )
        {
            game.setProposedDrawStatus (
                ProposedDrawType::ThreeFoldRepetition, Color::White, true
            );
            CHECK( game.status() == GameStatus::ThreefoldRepetitionReached );

            game.setProposedDrawStatus (
                ProposedDrawType::ThreeFoldRepetition, Color::Black, false
            );
            CHECK( game.status() == GameStatus::ThreefoldRepetitionAccepted );
        }

        SUBCASE( "Declined, play goes on until the fifth repetition" )
        {
            game.setProposedDrawStatus (
                ProposedDrawType::ThreeFoldRepetition,
                { DrawStatus::Declined, DrawStatus::Declined }
            );
            CHECK( game.status() == GameStatus::Playing );

            shuffleKnights (game, 1);
            CHECK( game.status() == GameStatus::Playing );

            shuffleKnights (game, 1);
            CHECK( game.status() == GameStatus::FivefoldRepetitionDraw );
        }
    }

    SUBCASE( "Fifty moves without progress" )
    {
        auto before = Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 99 80");
        CHECK( before.status() == GameStatus::Playing );

        before.move (moveParse ("a1 a2", Color::White));
        CHECK( before.status() == GameStatus::FiftyMovesWithoutProgressReached );

        SUBCASE( "Accepted by both players" )
        {
            before.setProposedDrawStatus (
                ProposedDrawType::FiftyMovesWithoutProgress,
                { DrawStatus::Accepted, DrawStatus::Accepted }
            );

            CHECK( before.status() == GameStatus::FiftyMovesWithoutProgressAccepted );
        }

        SUBCASE( "Declined" )
        {
            before.setProposedDrawStatus (
                ProposedDrawType::FiftyMovesWithoutProgress,
                { DrawStatus::Declined, DrawStatus::Declined }
            );

            CHECK( before.status() == GameStatus::Playing );
        }
    }

    SUBCASE( "Seventy-five moves without progress end the game even when declined" )
    {
        auto game = Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 149 110");
        game.setProposedDrawStatus (
            ProposedDrawType::FiftyMovesWithoutProgress,
            { DrawStatus::Declined, DrawStatus::Declined }
        );
        CHECK( game.status() == GameStatus::Playing );

        game.move (moveParse ("a1 a2", Color::White));
        CHECK( game.status() == GameStatus::SeventyFiveMovesWithoutProgressDraw );
    }

    SUBCASE( "A capture resets the fifty-move count" )
    {
        auto game = Game::createGameFromFen ("4k3/8/8/8/8/8/r7/R3K3 w - - 99 80");

        game.move (moveParse ("a1xa2", Color::White));

        CHECK( game.status() == GameStatus::Playing );
    }

    SUBCASE( "Checkmate takes precedence over a draw by the move count" )
    {
        auto game = Game::createGameFromFen ("6k1/5ppp/8/8/8/8/8/R3K3 w - - 99 80");

        game.move (moveParse ("a1 a8", Color::White));

        CHECK( game.status() == GameStatus::Checkmate );
    }
}

TEST_CASE( "Game::computerWantsDraw" )
{
    SUBCASE( "Not in a level position" )
    {
        auto game = Game::createStandardGame();

        CHECK( !game.computerWantsDraw (Color::White) );
        CHECK( !game.computerWantsDraw (Color::Black) );
    }

    SUBCASE( "Only the player who is a queen down" )
    {
        auto game = Game::createGameFromFen ("3qk3/8/8/8/8/8/8/4K3 w - - 0 1");

        CHECK( game.computerWantsDraw (Color::White) );
        CHECK( !game.computerWantsDraw (Color::Black) );
    }
}
