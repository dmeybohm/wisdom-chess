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
            .maxDepth = MaxDepth { 1 },
            .maxTime = std::chrono::seconds { 5 },
        };
        return ChessGame::fromFen (Fifty_Moves_Reached, config);
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
};

QTEST_GUILESS_MAIN( ChessEngineTest )

#include "chess_engine_test.moc"
