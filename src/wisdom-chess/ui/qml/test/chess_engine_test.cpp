#include <QSignalSpy>
#include <QTest>

#include "wisdom-chess/ui/qml/main/chess_engine.hpp"
#include "wisdom-chess/ui/qml/main/chess_game.hpp"

using namespace wisdom::ui::qml;

using wisdom::Color;
using wisdom::Player;
using wisdom::ProposedDrawType;

namespace
{
    // A rook endgame with the fifty-move rule reached: whoever moves is
    // offered a draw.
    constexpr auto Fifty_Moves_Reached = "4k3/8/8/8/8/8/8/R3K3 w - - 100 80";

    auto
    makeGame (Player white, Player black)
        -> std::shared_ptr<ChessGame>
    {
        auto config = ChessGame::Config {
            .players = { white, black },
            .searchDepth = 1,
            .thinkingTime = 5,
        };
        return ChessGame::fromFen (Fifty_Moves_Reached, config);
    }

    const ChessGame::Config Quick_Search {
        .players = { Player::ChessEngine, Player::Human },
        .searchDepth = 1,
        .thinkingTime = 5,
    };

    // The starting position, with the engine to move.
    auto
    makeStartingGame()
        -> std::shared_ptr<ChessGame>
    {
        return ChessGame::fromPlayers (Player::ChessEngine, Player::Human, Quick_Search);
    }

    void ignoreTimer ([[maybe_unused]] wisdom::nonnull<wisdom::MoveTimer> timer)
    {
    }
}

class ChessEngineTest : public QObject
{
    Q_OBJECT

private slots:
    // The engine answers for itself and, when it plays itself, for its
    // opponent too, the side to move first. White, a rook up, declines;
    // Black accepts, and that ends the game.
    void twoEnginesBothAnswerADrawProposal()
    {
        ChessEngine engine { makeGame (Player::ChessEngine, Player::ChessEngine), 1 };
        QSignalSpy answers { &engine, &ChessEngine::updateDrawStatus };
        QSignalSpy moves { &engine, &ChessEngine::engineMoved };

        engine.init();

        QCOMPARE( answers.count(), 2 );
        QCOMPARE( answers.at (0).at (1).value<Color>(), Color::White );
        QCOMPARE( answers.at (1).at (1).value<Color>(), Color::Black );
        QCOMPARE( answers.at (0).at (0).value<ProposedDrawType>(),
                  ProposedDrawType::FiftyMovesWithoutProgress );

        QVERIFY( !answers.at (0).at (2).toBool() );
        QVERIFY( answers.at (1).at (2).toBool() );
        QCOMPARE( moves.count(), 0 );
    }

    void againstAHumanOnlyTheEngineAnswers()
    {
        ChessEngine engine { makeGame (Player::ChessEngine, Player::Human), 1 };
        QSignalSpy answers { &engine, &ChessEngine::updateDrawStatus };
        QSignalSpy moves { &engine, &ChessEngine::engineMoved };

        engine.init();

        QCOMPARE( answers.count(), 1 );
        QCOMPARE( answers.at (0).at (1).value<Color>(), Color::White );

        // The human has yet to answer, so the engine waits.
        QCOMPARE( moves.count(), 0 );

        engine.receiveDrawStatus (ProposedDrawType::FiftyMovesWithoutProgress, Color::Black, false);
        QCOMPARE( moves.count(), 1 );
    }

    // A search depth out of range breaks a precondition of the settings.
    void aSlotThatThrowsReportsTheFailure()
    {
        ChessEngine engine { makeStartingGame(), 7 };
        QSignalSpy failures { &engine, &ChessEngine::engineFailed };

        auto out_of_range = Quick_Search;
        out_of_range.searchDepth = wisdom::ui::GameSettings::Max_Search_Depth + 1;
        engine.updateConfig (out_of_range, ignoreTimer);

        QCOMPARE( failures.count(), 1 );

        auto message = failures.at (0).at (0).toString();
        QVERIFY( message.contains (QStringLiteral ("Precondition failed")) );
        QCOMPARE( failures.at (0).at (1).toInt(), 7 );
    }

    void afterAFailureTheEngineDoesNothingUntilANewGame()
    {
        ChessEngine engine { makeStartingGame(), 1 };
        QSignalSpy failures { &engine, &ChessEngine::engineFailed };
        QSignalSpy moves { &engine, &ChessEngine::engineMoved };

        auto out_of_range = Quick_Search;
        out_of_range.searchDepth = wisdom::ui::GameSettings::Max_Search_Depth + 1;
        engine.updateConfig (out_of_range, ignoreTimer);
        QCOMPARE( failures.count(), 1 );

        // It is the engine's turn, but it stays stopped.
        engine.init();
        engine.updateConfig (Quick_Search, ignoreTimer);
        QCOMPARE( moves.count(), 0 );
        QCOMPARE( failures.count(), 1 );

        engine.reloadGame (makeStartingGame(), 2);
        QCOMPARE( moves.count(), 1 );
        QCOMPARE( moves.at (0).at (2).toInt(), 2 );
        QCOMPARE( failures.count(), 1 );
    }
};

QTEST_GUILESS_MAIN( ChessEngineTest )

#include "chess_engine_test.moc"
