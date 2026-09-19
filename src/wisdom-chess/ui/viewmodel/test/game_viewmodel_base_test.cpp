#include <doctest/doctest.h>

#include "wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp"

using namespace wisdom;
using wisdom::ui::DrawByRepetitionStatus;
using wisdom::ui::GameViewModelBase;

namespace
{
    // Owns a game and records which change callbacks ran.
    class TestViewModel : public GameViewModelBase
    {
    public:
        explicit TestViewModel (Game game)
            : my_game { std::move (game) }
        {
        }

        using GameViewModelBase::resetStateForNewGame;
        using GameViewModelBase::setCurrentTurn;
        using GameViewModelBase::setMoveStatus;
        using GameViewModelBase::setProposedDrawStatus;
        using GameViewModelBase::updateDisplayedGameState;

        [[nodiscard]] auto
        game()
            -> Game&
        {
            return my_game;
        }

        [[nodiscard]] auto
        countOf (const string& change) const
            -> std::ptrdiff_t
        {
            return std::count (changes.begin(), changes.end(), change);
        }

        vector<string> changes;

    protected:
        [[nodiscard]] auto
        getGame()
            -> observer_ptr<Game> override
        {
            return &my_game;
        }

        [[nodiscard]] auto
        getGame() const
            -> observer_ptr<const Game> override
        {
            return &my_game;
        }

        void onInCheckChanged() override { changes.emplace_back ("inCheck"); }
        void onMoveStatusChanged() override { changes.emplace_back ("moveStatus"); }
        void onGameOverStatusChanged() override { changes.emplace_back ("gameOverStatus"); }
        void onCurrentTurnChanged() override { changes.emplace_back ("currentTurn"); }
        void onThirdRepetitionDrawStatusChanged() override { changes.emplace_back ("thirdRepetition"); }
        void onFiftyMovesDrawStatusChanged() override { changes.emplace_back ("fiftyMoves"); }
        void onDisplayedGameStateUpdated() override { changes.emplace_back ("updated"); }

    private:
        Game my_game;
    };

    class PlainTextViewModel : public TestViewModel
    {
    public:
        using TestViewModel::TestViewModel;

    protected:
        [[nodiscard]] auto
        formatBold (const string& text) const
            -> string override
        {
            return text;
        }
    };

    constexpr auto Humans = Players { Player::Human, Player::Human };
    constexpr auto Engines = Players { Player::ChessEngine, Player::ChessEngine };

    constexpr auto Fools_Mate = "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3";

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

TEST_CASE( "A new view-model has nothing to show" )
{
    TestViewModel view_model { Game::createGame (Humans) };

    CHECK( !view_model.inCheck() );
    CHECK( view_model.moveStatus().empty() );
    CHECK( view_model.gameOverStatus().empty() );
    CHECK( view_model.currentTurn() == Color::White );
    CHECK( view_model.thirdRepetitionDrawStatus() == DrawByRepetitionStatus::NotReached );
    CHECK( view_model.fiftyMovesDrawStatus() == DrawByRepetitionStatus::NotReached );
    CHECK( view_model.changes.empty() );
}

TEST_CASE( "Updating the displayed state of a game" )
{
    SUBCASE( "A game in progress changes nothing, and says it has updated" )
    {
        TestViewModel view_model { Game::createGame (Humans) };

        view_model.updateDisplayedGameState();

        CHECK( view_model.changes == vector<string> { "updated" } );
    }

    SUBCASE( "Check" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1", Humans)
        };

        view_model.updateDisplayedGameState();

        CHECK( view_model.inCheck() );
        CHECK( view_model.gameOverStatus().empty() );
        CHECK( view_model.countOf ("inCheck") == 1 );
    }

    SUBCASE( "Check is shown for Black as well" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("4k3/4R3/8/8/8/8/8/4K3 b - - 0 1", Humans)
        };

        view_model.updateDisplayedGameState();

        CHECK( view_model.inCheck() );
    }

    SUBCASE( "Checkmate names the winner" )
    {
        TestViewModel view_model { Game::createGameFromFen (Fools_Mate, Humans) };

        view_model.updateDisplayedGameState();

        CHECK( view_model.gameOverStatus()
               == "<strong>Checkmate</strong> - Black wins the game." );
        CHECK( view_model.inCheck() );
    }

    SUBCASE( "Stalemate names the player without moves" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", Humans)
        };

        view_model.updateDisplayedGameState();

        CHECK( view_model.gameOverStatus()
               == "<strong>Stalemate</strong> - No legal moves for <strong>Black</strong>" );
        CHECK( !view_model.inCheck() );
    }

    SUBCASE( "Insufficient material" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("4k3/8/8/8/8/8/8/4K3 w - - 0 1", Humans)
        };

        view_model.updateDisplayedGameState();

        CHECK( view_model.gameOverStatus()
               == "<strong>Draw</strong> - Insufficient material to checkmate." );
    }

    SUBCASE( "Seventy-five moves without progress" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 150 110", Humans)
        };
        view_model.game().setProposedDrawStatus (
            ProposedDrawType::FiftyMovesWithoutProgress,
            { DrawStatus::Declined, DrawStatus::Declined }
        );

        view_model.updateDisplayedGameState();

        CHECK( view_model.gameOverStatus()
               == "<strong>Draw</strong> - Seventy-five moves without progress." );
    }

    SUBCASE( "formatBold() decides how the emphasis is written" )
    {
        PlainTextViewModel view_model { Game::createGameFromFen (Fools_Mate, Humans) };

        view_model.updateDisplayedGameState();

        CHECK( view_model.gameOverStatus() == "Checkmate - Black wins the game." );
    }

    SUBCASE( "A status that no longer applies is cleared" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("4k3/8/8/8/8/8/4r3/4K3 w - - 0 1", Humans)
        };
        view_model.setMoveStatus ("Illegal move");
        view_model.updateDisplayedGameState();
        REQUIRE( view_model.inCheck() );

        view_model.game().move (moveParse ("e1xe2", Color::White));
        view_model.updateDisplayedGameState();

        CHECK( !view_model.inCheck() );
        CHECK( view_model.moveStatus().empty() );
        CHECK( view_model.gameOverStatus()
               == "<strong>Draw</strong> - Insufficient material to checkmate." );
    }
}

TEST_CASE( "Draw proposals" )
{
    SUBCASE( "Threefold repetition is proposed to a human player" )
    {
        TestViewModel view_model { Game::createGame (Humans) };
        shuffleKnights (view_model.game(), 2);

        view_model.updateDisplayedGameState();

        CHECK( view_model.thirdRepetitionDrawStatus() == DrawByRepetitionStatus::Proposed );
        CHECK( view_model.fiftyMovesDrawStatus() == DrawByRepetitionStatus::NotReached );
        CHECK( view_model.gameOverStatus().empty() );
        CHECK( view_model.countOf ("thirdRepetition") == 1 );
    }

    SUBCASE( "Nothing is proposed when two engines play" )
    {
        TestViewModel view_model { Game::createGame (Engines) };
        shuffleKnights (view_model.game(), 2);

        view_model.updateDisplayedGameState();

        CHECK( view_model.thirdRepetitionDrawStatus() == DrawByRepetitionStatus::NotReached );
    }

    SUBCASE( "An accepted threefold repetition ends the game" )
    {
        TestViewModel view_model { Game::createGame (Humans) };
        shuffleKnights (view_model.game(), 2);
        view_model.updateDisplayedGameState();

        view_model.setProposedDrawStatus (
            ProposedDrawType::ThreeFoldRepetition, DrawByRepetitionStatus::Accepted
        );

        CHECK( view_model.game().status() == GameStatus::ThreefoldRepetitionAccepted );
        CHECK( view_model.gameOverStatus() == "<strong>Draw</strong> - Threefold repetition rule." );
    }

    SUBCASE( "A declined threefold repetition lets play go on to the fifth" )
    {
        TestViewModel view_model { Game::createGame (Humans) };
        shuffleKnights (view_model.game(), 2);
        view_model.updateDisplayedGameState();

        view_model.setProposedDrawStatus (
            ProposedDrawType::ThreeFoldRepetition, DrawByRepetitionStatus::Declined
        );

        CHECK( view_model.game().status() == GameStatus::Playing );
        CHECK( view_model.gameOverStatus().empty() );

        shuffleKnights (view_model.game(), 2);
        view_model.updateDisplayedGameState();

        CHECK( view_model.gameOverStatus() == "<strong>Draw</strong> - Fivefold repetition rule." );
    }

    SUBCASE( "Against an engine, only the human's answer is recorded" )
    {
        TestViewModel view_model {
            Game::createGame (Player::Human, Player::ChessEngine)
        };
        shuffleKnights (view_model.game(), 2);
        view_model.updateDisplayedGameState();

        view_model.setProposedDrawStatus (
            ProposedDrawType::ThreeFoldRepetition, DrawByRepetitionStatus::Accepted
        );

        // The engine has yet to answer for itself.
        CHECK( view_model.game().status() == GameStatus::ThreefoldRepetitionReached );

        view_model.game().setProposedDrawStatus (
            ProposedDrawType::ThreeFoldRepetition, Color::Black, false
        );
        CHECK( view_model.game().status() == GameStatus::ThreefoldRepetitionAccepted );
    }

    SUBCASE( "Fifty moves without progress is proposed and can be accepted" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 100 80", Humans)
        };

        view_model.updateDisplayedGameState();

        CHECK( view_model.fiftyMovesDrawStatus() == DrawByRepetitionStatus::Proposed );
        CHECK( view_model.thirdRepetitionDrawStatus() == DrawByRepetitionStatus::NotReached );

        view_model.setProposedDrawStatus (
            ProposedDrawType::FiftyMovesWithoutProgress, DrawByRepetitionStatus::Accepted
        );

        CHECK( view_model.gameOverStatus()
               == "<strong>Draw</strong> - Fifty moves without progress." );
    }
}

TEST_CASE( "Change callbacks run only for a change" )
{
    TestViewModel view_model { Game::createGame (Humans) };

    view_model.setMoveStatus ("Illegal move");
    view_model.setMoveStatus ("Illegal move");
    view_model.setCurrentTurn (Color::White);
    view_model.setCurrentTurn (Color::Black);
    view_model.setCurrentTurn (Color::Black);

    CHECK( view_model.changes == vector<string> { "moveStatus", "currentTurn" } );
    CHECK( view_model.moveStatus() == "Illegal move" );
    CHECK( view_model.currentTurn() == Color::Black );
}

TEST_CASE( "Resetting the state for a new game" )
{
    TestViewModel view_model { Game::createGameFromFen (Fools_Mate, Humans) };
    view_model.updateDisplayedGameState();
    view_model.setMoveStatus ("Illegal move");
    view_model.setCurrentTurn (Color::Black);
    view_model.changes.clear();

    view_model.resetStateForNewGame();

    CHECK( !view_model.inCheck() );
    CHECK( view_model.moveStatus().empty() );
    CHECK( view_model.gameOverStatus().empty() );
    CHECK( view_model.currentTurn() == Color::White );
    CHECK( view_model.changes
           == vector<string> { "inCheck", "moveStatus", "gameOverStatus", "currentTurn" } );
}

TEST_CASE( "Resetting clears the draw proposals" )
{
    TestViewModel view_model {
        Game::createGameFromFen ("4k3/8/8/8/8/8/8/R3K3 w - - 100 80", Humans)
    };
    view_model.updateDisplayedGameState();
    REQUIRE( view_model.fiftyMovesDrawStatus() == DrawByRepetitionStatus::Proposed );

    view_model.resetStateForNewGame();

    CHECK( view_model.fiftyMovesDrawStatus() == DrawByRepetitionStatus::NotReached );
    CHECK( view_model.thirdRepetitionDrawStatus() == DrawByRepetitionStatus::NotReached );
}

TEST_CASE( "GameViewModelBase::isLegalMove" )
{
    SUBCASE( "A human player's legal and illegal moves" )
    {
        TestViewModel view_model { Game::createGame (Humans) };

        CHECK( view_model.isLegalMove (moveParse ("e2 e4", Color::White)) );
        CHECK( !view_model.isLegalMove (moveParse ("e2 e5", Color::White)) );
        CHECK( !view_model.isLegalMove (moveParse ("e7 e5", Color::Black)) );
    }

    SUBCASE( "A move that leaves the king in check is not legal" )
    {
        TestViewModel view_model {
            Game::createGameFromFen ("k3r3/8/8/8/8/8/4R3/4K3 w - - 0 1", Humans)
        };

        CHECK( !view_model.isLegalMove (moveParse ("e2 d2", Color::White)) );
        CHECK( view_model.isLegalMove (moveParse ("e2 e5", Color::White)) );
    }

    SUBCASE( "No move is legal for the human while it is the engine's turn" )
    {
        TestViewModel view_model { Game::createGame (Player::ChessEngine, Player::Human) };

        CHECK( !view_model.isLegalMove (moveParse ("e2 e4", Color::White)) );
    }
}

TEST_CASE( "GameViewModelBase::needsPawnPromotion" )
{
    TestViewModel view_model {
        Game::createGameFromFen ("1n2k3/P7/8/8/8/8/4P3/4K3 w - - 0 1", Humans)
    };

    auto needs_promotion = [&view_model] (const char* src_text, const char* dst_text)
    {
        auto src = coordParse (src_text);
        auto dst = coordParse (dst_text);
        return view_model.needsPawnPromotion (src.row(), src.column(), dst.row(), dst.column());
    };

    CHECK( needs_promotion ("a7", "a8") );
    CHECK( needs_promotion ("a7", "b8") );
    CHECK( !needs_promotion ("e2", "e4") );
    CHECK( !needs_promotion ("e1", "d1") );
    CHECK( !needs_promotion ("c3", "c4") );
}
