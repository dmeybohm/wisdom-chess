#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

#include <map>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/ui/qml/main/pieces_model.hpp"

using wisdom::Color;
using wisdom::Move;
using wisdom::moveParse;
using wisdom::Player;

namespace
{
    // The image shown on each occupied square, keyed by (row, column).
    using Squares = std::map<std::pair<int, int>, QString>;

    auto
    makeConfig()
        -> ChessGame::Config
    {
        return ChessGame::Config {
            .players = { Player::Human, Player::Human },
            .maxDepth = MaxDepth { 2 },
            .maxTime = std::chrono::seconds { 1 },
        };
    }

    auto
    roleOf (const PiecesModel& model, int list_row, PiecesModel::Roles role)
        -> QVariant
    {
        return model.data (model.index (list_row, 0), role);
    }

    auto
    squaresOf (const PiecesModel& model)
        -> Squares
    {
        Squares result;
        for (int i = 0; i < model.rowCount ({}); i++)
        {
            auto square = std::pair {
                roleOf (model, i, PiecesModel::RowRole).toInt(),
                roleOf (model, i, PiecesModel::ColumnRole).toInt(),
            };
            result[square] = roleOf (model, i, PiecesModel::PieceImageRole).toString();
        }
        return result;
    }

    // What a model freshly filled from the game's board shows.
    auto
    squaresOf (const ChessGame& game)
        -> Squares
    {
        PiecesModel fresh_model;
        fresh_model.newGame (&game);
        return squaresOf (fresh_model);
    }

    auto
    listRowAt (const PiecesModel& model, const char* coord_text)
        -> int
    {
        auto coord = wisdom::coordParse (coord_text);
        for (int i = 0; i < model.rowCount ({}); i++)
        {
            if (roleOf (model, i, PiecesModel::RowRole).toInt() == coord.row<int>()
                && roleOf (model, i, PiecesModel::ColumnRole).toInt() == coord.column<int>())
            {
                return i;
            }
        }
        return -1;
    }

    // A game, a model that follows it move by move, and a consistency
    // tester that fails the test on any malformed model signal.
    struct Fixture
    {
        explicit Fixture (std::unique_ptr<ChessGame> the_game)
            : game { std::move (the_game) }
            , tester { &model, QAbstractItemModelTester::FailureReportingMode::QtTest }
        {
            model.newGame (game.get());
        }

        Fixture()
            : Fixture { ChessGame::fromPlayers (Player::Human, Player::Human, makeConfig()) }
        {
        }

        explicit Fixture (const char* fen)
            : Fixture { ChessGame::fromFen (fen, makeConfig()) }
        {
        }

        void play (const char* move_text)
        {
            auto who = game->state()->getCurrentTurn();
            auto move = moveParse (move_text, who);

            game->state()->move (move);
            model.playerMoved (move, who);
        }

        // Every piece is on a square of its own and the model shows
        // exactly what is on the board.
        [[nodiscard]] auto
        matchesBoard() const
            -> bool
        {
            auto shown = squaresOf (model);
            return shown.size() == static_cast<std::size_t> (model.rowCount ({}))
                && shown == squaresOf (*game);
        }

        std::unique_ptr<ChessGame> game;
        PiecesModel model;
        QAbstractItemModelTester tester;
    };
}

class PiecesModelTest : public QObject
{
    Q_OBJECT

private slots:
    void anEmptyModelHasNoRows()
    {
        PiecesModel model;

        QCOMPARE (model.rowCount ({}), 0);
    }

    void roleNamesAreWhatTheQmlDelegatesUse()
    {
        PiecesModel model;
        auto names = model.roleNames();

        QCOMPARE (names[PiecesModel::RowRole], QByteArray { "row" });
        QCOMPARE (names[PiecesModel::ColumnRole], QByteArray { "column" });
        QCOMPARE (names[PiecesModel::PieceImageRole], QByteArray { "pieceImage" });
        QCOMPARE (names[PiecesModel::IsCastlingRookRole], QByteArray { "isCastlingRook" });
        QCOMPARE (names[PiecesModel::CastlingSourceColumnRole],
                  QByteArray { "castlingSourceColumn" });
    }

    void aNewGameShowsTheStartingPosition()
    {
        Fixture fixture;

        QCOMPARE (fixture.model.rowCount ({}), 32);
        QVERIFY (fixture.matchesBoard());

        auto squares = squaresOf (fixture.model);
        QCOMPARE (squares.at ({ 7, 4 }), QStringLiteral ("../images/Chess_klt45.svg"));
        QCOMPARE (squares.at ({ 0, 3 }), QStringLiteral ("../images/Chess_qdt45.svg"));
        QCOMPARE (squares.at ({ 6, 0 }), QStringLiteral ("../images/Chess_plt45.svg"));
    }

    void aListItemHasNoChildRows()
    {
        Fixture fixture;

        QCOMPARE (fixture.model.rowCount (fixture.model.index (0, 0)), 0);
    }

    void anInvalidIndexOrAnUnknownRoleHasNoData()
    {
        Fixture fixture;

        QVERIFY (!fixture.model.data (QModelIndex {}, PiecesModel::RowRole).isValid());
        QVERIFY (!fixture.model.data (fixture.model.index (0, 0), Qt::DisplayRole).isValid());
    }

    void noPieceStartsAsACastlingRook()
    {
        Fixture fixture;

        for (int i = 0; i < fixture.model.rowCount ({}); i++)
        {
            QCOMPARE (roleOf (fixture.model, i, PiecesModel::IsCastlingRookRole).toBool(), false);
            QCOMPARE (roleOf (fixture.model, i, PiecesModel::CastlingSourceColumnRole).toInt(), -1);
        }
    }

    void aPlainMoveChangesOneRowInPlace()
    {
        Fixture fixture;
        auto list_row = listRowAt (fixture.model, "e2");
        QSignalSpy changed { &fixture.model, &PiecesModel::dataChanged };
        QSignalSpy removed { &fixture.model, &PiecesModel::rowsRemoved };

        fixture.play ("e2 e4");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 32);
        QCOMPARE (listRowAt (fixture.model, "e4"), list_row);
        QCOMPARE (removed.count(), 0);
        QCOMPARE (changed.count(), 1);

        auto arguments = changed.takeFirst();
        QCOMPARE (arguments.at (0).toModelIndex().row(), list_row);
        auto roles = arguments.at (2).value<QList<int>>();
        QVERIFY (roles.contains (PiecesModel::RowRole));
        QVERIFY (roles.contains (PiecesModel::ColumnRole));
        QVERIFY (!roles.contains (PiecesModel::PieceImageRole));
    }

    // The model lists pieces from the eighth rank down, so a White capture
    // removes a row before the mover's and a Black capture one after it.
    void capturesInBothListOrders()
    {
        Fixture fixture;
        QSignalSpy removed { &fixture.model, &PiecesModel::rowsRemoved };

        fixture.play ("e2 e4");
        fixture.play ("d7 d5");
        fixture.play ("e4xd5");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 31);
        QCOMPARE (removed.count(), 1);

        fixture.play ("d8xd5");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 30);
        QCOMPARE (removed.count(), 2);
    }

    // The mover follows the captured piece in the list, so it shifts into
    // the removed row's place and must still be visited.
    void theMoverDirectlyAfterTheCapturedPieceStillMoves()
    {
        Fixture fixture { "4k3/8/8/3pR3/8/8/8/4K3 w - - 0 1" };
        QCOMPARE (listRowAt (fixture.model, "e5"), listRowAt (fixture.model, "d5") + 1);

        fixture.play ("e5xd5");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 3);
        QCOMPARE (listRowAt (fixture.model, "e5"), -1);
    }

    void castling_data()
    {
        QTest::addColumn<QString> ("fen");
        QTest::addColumn<QString> ("move");
        QTest::addColumn<QString> ("rookSquare");
        QTest::addColumn<int> ("rookSourceColumn");

        QTest::newRow ("white kingside")
            << "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1" << "o-o" << "f1" << 7;
        QTest::newRow ("white queenside")
            << "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1" << "o-o-o" << "d1" << 0;
        QTest::newRow ("black kingside")
            << "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1" << "o-o" << "f8" << 7;
        QTest::newRow ("black queenside")
            << "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1" << "o-o-o" << "d8" << 0;
    }

    void castling()
    {
        QFETCH (QString, fen);
        QFETCH (QString, move);
        QFETCH (QString, rookSquare);
        QFETCH (int, rookSourceColumn);

        Fixture fixture { fen.toUtf8().constData() };

        fixture.play (move.toUtf8().constData());

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 6);

        // Only the castled rook carries the roles that drive its animation.
        auto rook_row = listRowAt (fixture.model, rookSquare.toUtf8().constData());
        QVERIFY (rook_row >= 0);
        for (int i = 0; i < fixture.model.rowCount ({}); i++)
        {
            auto is_rook = roleOf (fixture.model, i, PiecesModel::IsCastlingRookRole).toBool();
            auto source = roleOf (fixture.model, i, PiecesModel::CastlingSourceColumnRole).toInt();

            QCOMPARE (is_rook, i == rook_row);
            QCOMPARE (source, i == rook_row ? rookSourceColumn : -1);
        }
    }

    void theCastlingRolesAreClearedByTheNextMove()
    {
        Fixture fixture { "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1" };

        fixture.play ("o-o");
        fixture.play ("a8 a7");

        QVERIFY (fixture.matchesBoard());
        auto rook_row = listRowAt (fixture.model, "f1");
        QCOMPARE (roleOf (fixture.model, rook_row, PiecesModel::IsCastlingRookRole).toBool(), false);
        QCOMPARE (roleOf (fixture.model, rook_row, PiecesModel::CastlingSourceColumnRole).toInt(), -1);
    }

    // The captured pawn sits directly before the castled rook in the list,
    // so the rook shifts into the removed row's place. It must still be
    // visited, or it keeps the roles that replay its castling animation.
    void aCaptureBesideTheCastledRookStillClearsItsRoles()
    {
        Fixture fixture { "4k2r/8/8/8/8/8/7P/R3K3 w Q - 0 1" };
        QCOMPARE (listRowAt (fixture.model, "a1"), listRowAt (fixture.model, "h2") + 1);

        fixture.play ("o-o-o");
        auto rook_row = listRowAt (fixture.model, "d1");
        QCOMPARE (roleOf (fixture.model, rook_row, PiecesModel::IsCastlingRookRole).toBool(), true);

        fixture.play ("h8xh2");

        QVERIFY (fixture.matchesBoard());
        rook_row = listRowAt (fixture.model, "d1");
        QCOMPARE (roleOf (fixture.model, rook_row, PiecesModel::IsCastlingRookRole).toBool(), false);
        QCOMPARE (roleOf (fixture.model, rook_row, PiecesModel::CastlingSourceColumnRole).toInt(), -1);
    }

    void enPassantByWhite()
    {
        Fixture fixture;

        fixture.play ("e2 e4");
        fixture.play ("a7 a6");
        fixture.play ("e4 e5");
        fixture.play ("d7 d5");
        fixture.play ("e5 d6 ep");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 31);
        QCOMPARE (listRowAt (fixture.model, "d5"), -1);
    }

    void enPassantByBlack()
    {
        Fixture fixture;

        fixture.play ("a2 a3");
        fixture.play ("d7 d5");
        fixture.play ("a3 a4");
        fixture.play ("d5 d4");
        fixture.play ("e2 e4");
        fixture.play ("d4 e3 ep");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 31);
        QCOMPARE (listRowAt (fixture.model, "e4"), -1);
    }

    void promotionChangesTheImage()
    {
        Fixture fixture { "1n2k3/P7/8/8/8/8/8/4K3 w - - 0 1" };
        QSignalSpy changed { &fixture.model, &PiecesModel::dataChanged };

        fixture.play ("a7 a8(Q)");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (squaresOf (fixture.model).at ({ 0, 0 }),
                  QStringLiteral ("../images/Chess_qlt45.svg"));
        QCOMPARE (changed.count(), 1);
        QVERIFY (changed.first().at (2).value<QList<int>>().contains (PiecesModel::PieceImageRole));
    }

    void promotionWithACapture()
    {
        Fixture fixture { "1n2k3/P7/8/8/8/8/8/4K3 w - - 0 1" };

        fixture.play ("a7xb8(N)");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (fixture.model.rowCount ({}), 3);
        QCOMPARE (squaresOf (fixture.model).at ({ 0, 1 }),
                  QStringLiteral ("../images/Chess_nlt45.svg"));
    }

    void promotionByBlack()
    {
        Fixture fixture { "4k3/8/8/8/8/8/p7/4K3 b - - 0 1" };

        fixture.play ("a2 a1(R)");

        QVERIFY (fixture.matchesBoard());
        QCOMPARE (squaresOf (fixture.model).at ({ 7, 0 }),
                  QStringLiteral ("../images/Chess_rdt45.svg"));
    }

    void aLongerGameStaysInStep()
    {
        Fixture fixture;
        const char* moves[] = {
            "e2 e4", "e7 e5", "g1 f3", "b8 c6", "f1 c4", "g8 f6", "o-o", "f6xe4",
            "d2 d4", "e5xd4", "f1 e1", "d7 d5", "c4xd5", "d8xd5", "b1 c3", "d5 a5",
            "c3xe4", "c8 e6", "e4 g5", "o-o-o",
        };

        for (auto move : moves)
        {
            fixture.play (move);
            QVERIFY2 (fixture.matchesBoard(), move);
        }
    }

    void aNewGameReplacesTheRows()
    {
        Fixture fixture;
        fixture.play ("e2 e4");
        fixture.play ("d7 d5");
        fixture.play ("e4xd5");
        QSignalSpy removed { &fixture.model, &PiecesModel::rowsRemoved };

        auto new_game = ChessGame::fromFen ("4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", makeConfig());
        fixture.model.newGame (new_game.get());

        QCOMPARE (removed.count(), 1);
        QCOMPARE (fixture.model.rowCount ({}), 3);
        QCOMPARE (squaresOf (fixture.model), squaresOf (*new_game));
    }
};

QTEST_GUILESS_MAIN (PiecesModelTest)

#include "pieces_model_test.moc"
