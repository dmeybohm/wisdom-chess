#include <QGuiApplication>
#include <QScreen>
#include <QTest>

#include "application_fixture.hpp"

using wisdom::Color;
using wisdom::ColoredPiece;
using wisdom::Piece;
using namespace wisdom::ui::test;

// The mobile QML is only part of the Android build. It is loaded here on the
// desktop, where Helper.isMobile() is false, so this shows that the files
// work, not how they look on a phone.
class MobileTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        wisdom::ui::registerQmlTypes();
    }

    void init()
    {
        my_app = std::make_unique<Application> ("main/mobile_main.qml");
        QVERIFY( my_app->window() != nullptr );

        my_app->window()->requestActivate();
        QVERIFY( QTest::qWaitForWindowActive (my_app->window()) );
        my_app->makeBothPlayersHuman();
    }

    void cleanup()
    {
        auto warnings = my_app->warnings;
        my_app.reset();

        QVERIFY2( warnings.isEmpty(), qPrintable (warnings.join (QLatin1Char ('\n'))) );
    }

    void theWindowShowsTheStartingPosition()
    {
        QCOMPARE( my_app->squares().size(), 64 );
        QCOMPARE( my_app->pieces().size(), 32 );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("<b>White</b> to move")) );

        for (auto coord : { "a1", "e1", "h8" })
            QVERIFY2( drawnOn (*my_app, my_app->pieceAt (coord), coord), coord );
    }

    void clickingTwoSquaresMovesAPiece()
    {
        auto* pawn = my_app->pieceAt ("e2");

        my_app->move ("e2", "e4");

        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
        QTRY_VERIFY( drawnOn (*my_app, pawn, "e4") );
    }

    // The handler once read a property that does not exist. Any error in it
    // is a QML warning, which cleanup() turns into a failure.
    void anOrientationChangeRecalculatesTheSquareSize()
    {
        auto size_before = my_app->squareSize();
        QVERIFY( size_before > 0 );

        auto* screen = my_app->window()->screen();
        emit screen->primaryOrientationChanged (Qt::LandscapeOrientation);
        QCoreApplication::processEvents();

        QCOMPARE( my_app->squareSize(), size_before );
        QCOMPARE( my_app->squares().size(), 64 );
    }

    void theMenuOpens()
    {
        auto tool_buttons = my_app->shownItemsOfClass ("QQuickToolButton");
        QCOMPARE( tool_buttons.size(), 1 );

        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) != nullptr );
        QVERIFY( my_app->buttonWithText (QStringLiteral ("Settings")) != nullptr );
    }

    // Pressing the button is a press outside the menu. It once closed the
    // menu on the press, and the click on release opened it again.
    void theMenuButtonClosesTheMenu()
    {
        auto tool_buttons = my_app->shownItemsOfClass ("QQuickToolButton");
        QCOMPARE( tool_buttons.size(), 1 );

        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) != nullptr );

        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) == nullptr );
    }

    void theMenuOpensBelowTheToolbarAtTheRightEdge()
    {
        auto tool_buttons = my_app->shownItemsOfClass ("QQuickToolButton");
        QCOMPARE( tool_buttons.size(), 1 );
        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) != nullptr );

        QVERIFY( QQuickTest::qWaitForPolish (my_app->window()) );
        auto* toolbar = ancestorOfClass (tool_buttons[0], "QQuickToolBar");
        QVERIFY( toolbar != nullptr );
        auto* menu = ancestorOfClass (
            my_app->buttonWithText (QStringLiteral ("New Game")), "QQuickPopupItem"
        );
        QVERIFY( menu != nullptr );

        auto top_right = menu->mapToScene (QPointF { menu->width(), 0 });
        QCOMPARE( top_right.x(), my_app->window()->width() );
        QVERIFY( top_right.y() >= toolbar->height() );
    }

    void aNewGameCanBeStartedFromTheMenu()
    {
        my_app->move ("e2", "e4");
        auto tool_buttons = my_app->shownItemsOfClass ("QQuickToolButton");
        QCOMPARE( tool_buttons.size(), 1 );
        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) != nullptr );
        my_app->clickItem (my_app->buttonWithText (QStringLiteral ("New Game")));

        auto* dialog = my_app->popupWithTitle (QStringLiteral ("New Game"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        QTRY_VERIFY( dialog->property ("opened").toBool() );
        QVERIFY( my_app->dialogFitsText (dialog, QStringLiteral ("Start a new game?")) );
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("Yes")) != nullptr );
        my_app->clickItem (my_app->buttonWithText (QStringLiteral ("Yes")));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QTRY_VERIFY( my_app->boardPieceAt ("e2") == ColoredPiece::make (Color::White, Piece::Pawn) );
        QTRY_VERIFY( my_app->piecesMatchTheBoard() );
    }

private:
    [[nodiscard]] static auto
    ancestorOfClass (QQuickItem* item, const char* class_name)
        -> QQuickItem*
    {
        while (item != nullptr && !item->inherits (class_name))
            item = item->parentItem();
        return item;
    }

    std::unique_ptr<Application> my_app;
};

QTEST_MAIN( MobileTest )

#include "mobile_test.moc"
