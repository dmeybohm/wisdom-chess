#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTest>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/ui/qml/main/game_model.hpp"
#include "wisdom-chess/ui/qml/main/pieces_model.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"

using wisdom::Color;
using wisdom::ColoredPiece;
using wisdom::Piece;

namespace
{
    // Lets the tests read the board the model holds.
    class InspectableGameModel : public GameModel
    {
    public:
        [[nodiscard]] auto
        board() const
            -> const wisdom::Board&
        {
            return getGame()->getBoard();
        }
    };

    // The application as main.cpp assembles it: both models, the signal
    // connections between them and the real QML, loaded from resources.
    class Application
    {
    public:
        Application()
        {
            QObject::connect (&game_model, &GameModel::engineMoved,
                              &pieces_model, &PiecesModel::playerMoved);
            QObject::connect (&game_model, &GameModel::humanMoved,
                              &pieces_model, &PiecesModel::playerMoved);
            QObject::connect (&game_model, &GameModel::gameStarted,
                              &pieces_model, &PiecesModel::newGame);

            QObject::connect (&my_engine, &QQmlEngine::warnings, &my_engine,
                [this] (const QList<QQmlError>& errors)
                {
                    for (const auto& error : errors)
                        warnings << error.toString();
                });

            auto* context = my_engine.rootContext();
            context->setContextProperty (QStringLiteral ("_myGameModel"), &game_model);
            context->setContextProperty (QStringLiteral ("_myPiecesModel"), &pieces_model);

            my_engine.load (QUrl { QStringLiteral ("qrc:/qt/qml/WisdomChess/main/desktop_main.qml") });
            game_model.start();
        }

        // The window's closing handler does this in the application. The
        // model must not be destroyed while its engine thread runs.
        ~Application()
        {
            game_model.applicationExiting();
        }

        Application (const Application&) = delete;
        auto operator= (const Application&) -> Application& = delete;

        [[nodiscard]] auto
        window() const
            -> QQuickWindow*
        {
            auto roots = my_engine.rootObjects();
            return roots.isEmpty() ? nullptr : qobject_cast<QQuickWindow*> (roots.first());
        }

        // Without this the engine starts thinking after White's first move.
        void makeBothPlayersHuman()
        {
            auto settings = game_model.cloneGameSettings();
            const auto& meta_object = GameSettings::staticMetaObject;
            auto property = meta_object.property (meta_object.indexOfProperty ("blackPlayer"));

            property.writeOnGadget (&settings, QVariant::fromValue (wisdom::ui::Player::Human));
            game_model.setGameSettings (settings);
        }

        [[nodiscard]] auto
        squareSize() const
            -> double
        {
            return window()->property ("squareSize").toDouble();
        }

        [[nodiscard]] auto
        squares() const
            -> QList<QQuickItem*>
        {
            return itemsWithProperties ("boardRow", "bgColor");
        }

        [[nodiscard]] auto
        pieces() const
            -> QList<QQuickItem*>
        {
            return itemsWithProperties ("isCastlingRook", "castlingSourceColumn");
        }

        [[nodiscard]] auto
        squareAt (const char* coord_text) const
            -> QQuickItem*
        {
            auto coord = wisdom::coordParse (coord_text);
            for (auto* square : squares())
            {
                if (square->property ("boardRow").toInt() == coord.row<int>()
                    && square->property ("boardColumn").toInt() == coord.column<int>())
                {
                    return square;
                }
            }
            return nullptr;
        }

        [[nodiscard]] auto
        pieceAt (const char* coord_text) const
            -> QQuickItem*
        {
            auto coord = wisdom::coordParse (coord_text);
            for (auto* piece : pieces())
            {
                if (piece->property ("row").toInt() == coord.row<int>()
                    && piece->property ("column").toInt() == coord.column<int>())
                {
                    return piece;
                }
            }
            return nullptr;
        }

        void click (const char* coord_text)
        {
            auto* square = squareAt (coord_text);
            auto center = square->mapToScene (QPointF { square->width() / 2, square->height() / 2 });
            QTest::mouseClick (window(), Qt::LeftButton, Qt::NoModifier, center.toPoint());
            QCoreApplication::processEvents();
        }

        // A move the way a player makes one: click the piece, click the target.
        void move (const char* src_text, const char* dst_text)
        {
            click (src_text);
            click (dst_text);
        }

        [[nodiscard]] auto
        boardPieceAt (const char* coord_text) const
            -> ColoredPiece
        {
            return game_model.board().pieceAt (wisdom::coordParse (coord_text));
        }

        InspectableGameModel game_model;
        PiecesModel pieces_model;
        QStringList warnings;

    private:
        [[nodiscard]] auto
        itemsWithProperties (const char* first, const char* second) const
            -> QList<QQuickItem*>
        {
            QList<QQuickItem*> result;
            collectItems (window()->contentItem(), first, second, result);
            return result;
        }

        // Walks the visual tree: a Repeater's delegates are child items of
        // its parent without being its QObject children.
        static void collectItems ( // NOLINT(misc-no-recursion)
            QQuickItem* item,
            const char* first,
            const char* second,
            QList<QQuickItem*>& result
        ) {
            if (item->property (first).isValid() && item->property (second).isValid())
                result << item;

            for (auto* child : item->childItems())
                collectItems (child, first, second, result);
        }

        QQmlApplicationEngine my_engine;
    };

    // Where the middle of an item is drawn, in window coordinates. The
    // middle stays meaningful when the board and the pieces are rotated.
    auto
    drawnAt (const QQuickItem* item)
        -> QPointF
    {
        return item->mapToScene (QPointF { item->width() / 2, item->height() / 2 });
    }

    auto
    drawnOn (const Application& app, const QQuickItem* piece, const char* coord_text)
        -> bool
    {
        auto expected = drawnAt (app.squareAt (coord_text));
        auto actual = drawnAt (piece);
        return qAbs (actual.x() - expected.x()) < 0.5 && qAbs (actual.y() - expected.y()) < 0.5;
    }
}

class ApplicationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        wisdom::ui::registerQmlTypes();
    }

    void init()
    {
        my_app = std::make_unique<Application>();
        QVERIFY( my_app->window() != nullptr );

        my_app->window()->requestActivate();
        QVERIFY( QTest::qWaitForWindowActive (my_app->window()) );
        my_app->makeBothPlayersHuman();
    }

    // No test may leave a QML warning behind: a ReferenceError or a
    // binding to something undefined fails whichever test provoked it.
    void cleanup()
    {
        auto warnings = my_app->warnings;
        my_app.reset();

        QVERIFY2( warnings.isEmpty(), qPrintable (warnings.join (QLatin1Char ('\n'))) );
    }

    void theWindowShowsTheStartingPosition()
    {
        QCOMPARE( my_app->window()->title(), QStringLiteral ("Wisdom Chess") );
        QCOMPARE( my_app->squares().size(), 64 );
        QCOMPARE( my_app->pieces().size(), 32 );

        for (auto coord : { "a1", "e1", "h1", "e2", "a8", "d8", "h7" })
        {
            auto* piece = my_app->pieceAt (coord);
            QVERIFY2( piece != nullptr, coord );
            QVERIFY2( drawnOn (*my_app, piece, coord), coord );
        }
    }

    void everyPieceImageLoads()
    {
        for (auto* piece : my_app->pieces())
        {
            // QQuickImage::Ready
            QTRY_COMPARE( piece->property ("status").toInt(), 1 );
        }
    }

    void clickingTwoSquaresMovesAPiece()
    {
        auto* pawn = my_app->pieceAt ("e2");

        my_app->move ("e2", "e4");

        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
        QCOMPARE( my_app->pieceAt ("e4"), pawn );
        QTRY_VERIFY( drawnOn (*my_app, pawn, "e4") );
        QVERIFY( my_app->game_model.qmlCurrentTurn() == wisdom::ui::Color::Black );
        QCOMPARE( my_app->game_model.qmlMoveStatus(), QString {} );
    }

    void anIllegalMoveIsReportedAndNothingMoves()
    {
        auto* pawn = my_app->pieceAt ("e2");

        my_app->move ("e2", "e5");

        QCOMPARE( my_app->game_model.qmlMoveStatus(), QStringLiteral ("Illegal move") );
        QCOMPARE( my_app->pieceAt ("e2"), pawn );
        QVERIFY( my_app->game_model.qmlCurrentTurn() == wisdom::ui::Color::White );

        my_app->move ("e2", "e4");

        QCOMPARE( my_app->game_model.qmlMoveStatus(), QString {} );
    }

    void aCaptureRemovesThePieceFromTheBoard()
    {
        my_app->move ("e2", "e4");
        my_app->move ("d7", "d5");
        auto* white_pawn = my_app->pieceAt ("e4");

        my_app->move ("e4", "d5");

        QCOMPARE( my_app->pieces().size(), 31 );
        QCOMPARE( my_app->pieceAt ("d5"), white_pawn );
        QTRY_VERIFY( drawnOn (*my_app, white_pawn, "d5") );
    }

    // The rook's castling animation starts from the rook's own square. It
    // must never be drawn anywhere but between there and its destination,
    // during castling or in the moves that follow.
    void castlingAnimatesTheRookAndLeavesItUsable()
    {
        my_app->move ("e2", "e4");
        my_app->move ("e7", "e5");
        my_app->move ("g1", "f3");
        my_app->move ("b8", "c6");
        my_app->move ("f1", "c4");
        my_app->move ("f8", "c5");
        auto* king = my_app->pieceAt ("e1");
        auto* rook = my_app->pieceAt ("h1");

        my_app->move ("e1", "g1");

        QVERIFY( my_app->boardPieceAt ("f1") == ColoredPiece::make (Color::White, Piece::Rook) );
        QCOMPARE( my_app->pieceAt ("g1"), king );
        QCOMPARE( my_app->pieceAt ("f1"), rook );

        auto f1 = drawnAt (my_app->squareAt ("f1"));
        auto h1 = drawnAt (my_app->squareAt ("h1"));
        auto watchRook = [&] (int milliseconds, double leftmost, double rightmost)
        {
            for (int elapsed = 0; elapsed < milliseconds; elapsed += 10)
            {
                auto at = drawnAt (rook);
                QVERIFY2( at.x() >= leftmost - 0.5 && at.x() <= rightmost + 0.5,
                          qPrintable (QStringLiteral ("rook drawn at x = %1").arg (at.x())) );
                QVERIFY( qAbs (at.y() - f1.y()) < 0.5 );
                QTest::qWait (10);
            }
        };

        watchRook (700, f1.x(), h1.x());
        if (QTest::currentTestFailed())
            return;
        QVERIFY( drawnOn (*my_app, rook, "f1") );
        QVERIFY( drawnOn (*my_app, king, "g1") );

        // The move after castling must not disturb the rook.
        my_app->move ("g8", "f6");
        watchRook (500, f1.x(), f1.x());
        if (QTest::currentTestFailed())
            return;

        // And the rook still goes where it is told afterwards.
        my_app->move ("f1", "e1");
        auto e1 = drawnAt (my_app->squareAt ("e1"));
        watchRook (500, e1.x(), f1.x());
        if (QTest::currentTestFailed())
            return;
        QCOMPARE( my_app->pieceAt ("e1"), rook );
        QVERIFY( drawnOn (*my_app, rook, "e1") );
    }

    void aPromotionGoesThroughTheDropdown()
    {
        my_app->move ("h2", "h4");
        my_app->move ("g7", "g5");
        my_app->move ("h4", "g5");
        my_app->move ("h7", "h6");
        my_app->move ("g5", "h6");
        my_app->move ("g8", "f6");
        my_app->move ("h6", "h7");
        my_app->move ("f6", "g8");
        auto* pawn = my_app->pieceAt ("h7");
        auto pawn_image = pawn->property ("source").toUrl();

        // Choosing the target only opens the dropdown.
        my_app->move ("h7", "g8");
        QVERIFY( my_app->boardPieceAt ("h7") == ColoredPiece::make (Color::White, Piece::Pawn) );

        // The dropdown hangs down from the target square, queen first and
        // knight last. One click highlights an entry, the next chooses it.
        my_app->click ("g5");
        QVERIFY( my_app->boardPieceAt ("h7") == ColoredPiece::make (Color::White, Piece::Pawn) );
        my_app->click ("g5");

        QVERIFY( my_app->boardPieceAt ("g8") == ColoredPiece::make (Color::White, Piece::Knight) );
        QVERIFY( my_app->boardPieceAt ("h7") == wisdom::Piece_And_Color_None );
        QCOMPARE( my_app->pieceAt ("g8"), pawn );
        QVERIFY( pawn->property ("source").toUrl() != pawn_image );
        QVERIFY( pawn->property ("source").toUrl().toString().endsWith (QStringLiteral ("Chess_nlt45.svg")) );
        QCOMPARE( my_app->pieces().size(), 29 );
        QVERIFY( my_app->game_model.qmlCurrentTurn() == wisdom::ui::Color::Black );
    }

    void checkmateIsAnnounced()
    {
        my_app->move ("f2", "f3");
        my_app->move ("e7", "e5");
        my_app->move ("g2", "g4");
        QCOMPARE( my_app->game_model.qmlGameOverStatus(), QString {} );

        my_app->move ("d8", "h4");

        QVERIFY( my_app->game_model.qmlGameOverStatus().contains (QStringLiteral ("Checkmate")) );
        QVERIFY( my_app->game_model.qmlInCheck() );

        // The game is over, so the board takes no more moves.
        my_app->move ("a2", "a3");
        QVERIFY( my_app->boardPieceAt ("a2") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    void checkIsShown()
    {
        my_app->move ("e2", "e4");
        my_app->move ("f7", "f6");

        my_app->move ("d1", "h5");

        QVERIFY( my_app->game_model.qmlInCheck() );
        QCOMPARE( my_app->game_model.qmlGameOverStatus(), QString {} );
    }

    void restartingPutsThePiecesBack()
    {
        my_app->move ("e2", "e4");
        my_app->move ("d7", "d5");
        my_app->move ("e4", "d5");
        QCOMPARE( my_app->pieces().size(), 31 );

        my_app->game_model.restart();

        QTRY_COMPARE( my_app->pieces().size(), 32 );
        QVERIFY( my_app->pieceAt ("e2") != nullptr );
        QVERIFY( my_app->pieceAt ("d7") != nullptr );
        QVERIFY( my_app->game_model.qmlCurrentTurn() == wisdom::ui::Color::White );

        // The new game is playable.
        my_app->move ("d2", "d4");
        QVERIFY( my_app->boardPieceAt ("d4") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    void flippingTheBoardTurnsItAround()
    {
        auto before = drawnAt (my_app->pieceAt ("a1"));

        auto settings = my_app->game_model.cloneUISettings();
        const auto& meta_object = UISettings::staticMetaObject;
        meta_object.property (meta_object.indexOfProperty ("flipped")).writeOnGadget (&settings, true);
        my_app->game_model.setUISettings (settings);

        // a1 ends up where h8 was drawn, seven squares across and up.
        auto distance = my_app->squareSize() * 7;
        QTRY_VERIFY( qAbs (drawnAt (my_app->pieceAt ("a1")).x() - (before.x() + distance)) < 1.0 );
        QTRY_VERIFY( qAbs (drawnAt (my_app->pieceAt ("a1")).y() - (before.y() - distance)) < 1.0 );

        // Squares turn with the board, so clicking still moves the right piece.
        my_app->move ("e2", "e4");
        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

private:
    std::unique_ptr<Application> my_app;
};

QTEST_MAIN( ApplicationTest )

#include "application_test.moc"
