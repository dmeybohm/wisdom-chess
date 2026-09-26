#include <QGuiApplication>
#include <QTest>

#include "application_fixture.hpp"

using wisdom::Color;
using wisdom::ColoredPiece;
using wisdom::Piece;
using namespace wisdom::ui::test;

// The wasm QML is only part of the WebAssembly build. It is loaded here on
// the desktop, where Platform.isWebAssembly is false, so this shows that the
// file works, not how it looks in a browser.
class WasmTest : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        my_app = std::make_unique<Application> ("main/wasm_main.qml");
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
    }

    void clickingTwoSquaresMovesAPiece()
    {
        my_app->move ("e2", "e4");

        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    // The toolbar has the rook icon and the arrow, both buttons that open
    // the menu; the title between them opens it through a MouseArea.
    void theMenuOpensFromTheToolbar()
    {
        auto tool_buttons = my_app->shownItemsOfClass ("QQuickToolButton");
        QCOMPARE( tool_buttons.size(), 2 );

        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) != nullptr );
        QVERIFY( my_app->buttonWithText (QStringLiteral ("Settings")) != nullptr );
    }

private:
    std::unique_ptr<Application> my_app;
};

QTEST_MAIN( WasmTest )

#include "wasm_test.moc"
