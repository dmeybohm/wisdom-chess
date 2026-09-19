#include <QSignalSpy>
#include <QTest>

#include "application_fixture.hpp"

using wisdom::Color;
using wisdom::ColoredPiece;
using wisdom::Piece;
using namespace wisdom::ui::test;

namespace
{
    constexpr int Settings_Apply_Delay = 350;
}

class DialogsTest : public QObject
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

    void cleanup()
    {
        auto warnings = my_app->warnings;
        my_app.reset();

        QVERIFY2( warnings.isEmpty(), qPrintable (warnings.join (QLatin1Char ('\n'))) );
    }

    void theStatusBarFollowsTheGame()
    {
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("<b>White</b> to move")) );

        my_app->move ("e2", "e4");
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("<b>Black</b> to move")) );

        my_app->move ("e7", "e4");
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("Illegal move")) );

        my_app->move ("f7", "f6");
        my_app->move ("d1", "h5");
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("Check!")) );
        QVERIFY( !my_app->showsText (QStringLiteral ("Illegal move")) );
    }

    void theStatusBarAnnouncesCheckmateInsteadOfTheTurn()
    {
        my_app->move ("f2", "f3");
        my_app->move ("e7", "e5");
        my_app->move ("g2", "g4");
        my_app->move ("d8", "h4");

        QTRY_VERIFY( my_app->showsText (QStringLiteral ("Checkmate")) );
        QVERIFY( !my_app->showsText (QStringLiteral ("to move")) );
        QVERIFY( !my_app->showsText (QStringLiteral ("Check!")) );
    }

    void theMenuOpensFromTheToolbar()
    {
        QVERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) == nullptr );

        openMenu();

        QVERIFY( my_app->buttonWithText (QStringLiteral ("New Game")) != nullptr );
        QVERIFY( my_app->buttonWithText (QStringLiteral ("Settings")) != nullptr );
        QVERIFY( my_app->buttonWithText (QStringLiteral ("About Wisdom Chess")) != nullptr );
        QVERIFY( my_app->buttonWithText (QStringLiteral ("Quit")) != nullptr );

        // Only the WebAssembly build links to the React version.
        QVERIFY( my_app->buttonWithText (QStringLiteral ("React Version")) == nullptr );
    }

    void aNewGameCanBeDeclined()
    {
        my_app->move ("e2", "e4");
        chooseFromMenu (QStringLiteral ("New Game"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("New Game"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("Start a new game?")) );

        clickButton (QStringLiteral ("No"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    void aNewGameCanBeStarted()
    {
        my_app->move ("e2", "e4");
        my_app->move ("d7", "d5");
        my_app->move ("e4", "d5");
        chooseFromMenu (QStringLiteral ("New Game"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("New Game"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        clickButton (QStringLiteral ("Yes"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QTRY_COMPARE( my_app->pieces().size(), 32 );
        QVERIFY( my_app->piecesMatchTheBoard() );
        QVERIFY( my_app->boardPieceAt ("e2") == ColoredPiece::make (Color::White, Piece::Pawn) );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("<b>White</b> to move")) );
    }

    void anOpenDialogKeepsClicksFromTheBoard()
    {
        chooseFromMenu (QStringLiteral ("New Game"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("New Game"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        my_app->move ("e2", "e4");

        QVERIFY( my_app->boardPieceAt ("e2") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    void theAboutDialogClosesWithItsButton()
    {
        chooseFromMenu (QStringLiteral ("About Wisdom Chess"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("About Wisdom Chess"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("Wisdom Chess ©")) );

        clickButton (QStringLiteral ("OK"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
    }

    void theAboutDialogClosesWithEscape()
    {
        chooseFromMenu (QStringLiteral ("About Wisdom Chess"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("About Wisdom Chess"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        QTest::keyClick (my_app->window(), Qt::Key_Escape);

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
    }

    void quittingAsksFirst()
    {
        QSignalSpy quit { &my_app->engine(), &QQmlEngine::quit };
        chooseFromMenu (QStringLiteral ("Quit"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Quit Wisdom Chess"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        clickButton (QStringLiteral ("No"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QCOMPARE( quit.count(), 0 );

        chooseFromMenu (QStringLiteral ("Quit"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        clickButton (QStringLiteral ("Yes"));

        QTRY_COMPARE( quit.count(), 1 );
    }

    void settingsAreAppliedAfterApply()
    {
        chooseFromMenu (QStringLiteral ("Settings"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Settings"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        // The first check box flips the board, the second is debug logging.
        auto check_boxes = my_app->shownItemsOfClass ("QQuickCheckBox");
        QCOMPARE( check_boxes.size(), 2 );
        my_app->clickItem (check_boxes[0]);
        my_app->clickItem (check_boxes[1]);

        // White's radio buttons come before Black's.
        auto computers = radioButtons (QStringLiteral ("Computer"));
        QCOMPARE( computers.size(), 2 );
        my_app->clickItem (computers[1]);

        QCOMPARE( my_app->game_model.uiSettings().flipped(), false );
        clickButton (QStringLiteral ("Apply"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QTRY_COMPARE( my_app->game_model.uiSettings().flipped(), true );
        QCOMPARE( my_app->game_model.gameSettings().debugLogging(), true );
        QVERIFY( my_app->game_model.gameSettings().blackPlayer() == wisdom::ui::Player::Computer );
        QVERIFY( my_app->game_model.gameSettings().whitePlayer() == wisdom::ui::Player::Human );
    }

    void cancelledSettingsAreDropped()
    {
        auto before = my_app->game_model.gameSettings();
        chooseFromMenu (QStringLiteral ("Settings"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Settings"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        auto check_boxes = my_app->shownItemsOfClass ("QQuickCheckBox");
        QCOMPARE( check_boxes.size(), 2 );
        my_app->clickItem (check_boxes[0]);
        my_app->clickItem (check_boxes[1]);

        clickButton (QStringLiteral ("Cancel"));
        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QTest::qWait (Settings_Apply_Delay + 150);

        QCOMPARE( my_app->game_model.uiSettings().flipped(), false );
        QVERIFY( my_app->game_model.gameSettings() == before );

        // Reopened, the dialog shows the settings in force, not the edits.
        chooseFromMenu (QStringLiteral ("Settings"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        for (auto* check_box : my_app->shownItemsOfClass ("QQuickCheckBox"))
            QCOMPARE( check_box->property ("checked").toBool(), false );
    }

    void theSettingsDialogShowsTheCurrentSettings()
    {
        chooseFromMenu (QStringLiteral ("Settings"));
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Settings"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        // init() made both players human.
        auto humans = radioButtons (QStringLiteral ("Human"));
        QCOMPARE( humans.size(), 2 );
        QCOMPARE( humans[0]->property ("checked").toBool(), true );
        QCOMPARE( humans[1]->property ("checked").toBool(), true );

        auto sliders = my_app->shownItemsOfClass ("QQuickSlider");
        QCOMPARE( sliders.size(), 2 );
        QCOMPARE( sliders[0]->property ("value").toInt(), wisdom::Default_Max_Search_Seconds );
        QCOMPARE( sliders[1]->property ("value").toInt(), wisdom::Default_Max_Depth / 2 );
    }

    void aDrawByRepetitionCanBeAccepted()
    {
        repeatThePositionThreeTimes();
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Draw Offer"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("repeated three times")) );

        clickButton (QStringLiteral ("Yes"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("Threefold repetition")) );

        // The game is over.
        my_app->move ("e2", "e4");
        QVERIFY( my_app->boardPieceAt ("e2") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    void aDrawByRepetitionCanBeDeclined()
    {
        repeatThePositionThreeTimes();
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Draw Offer"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );

        clickButton (QStringLiteral ("No"));

        QTRY_VERIFY( !dialog->property ("visible").toBool() );
        QCOMPARE( my_app->game_model.qmlGameOverStatus(), QString {} );

        // Play goes on, and the same offer is not made again.
        my_app->move ("e2", "e4");
        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
        QVERIFY( !dialog->property ("visible").toBool() );
    }

    // A dialog gives the focus back, when it closes, to whatever had it
    // when it opened. If that were a square, the board would take the
    // player's next click as the target of a move from there and the click
    // would be lost. A draw offer opens in the middle of handling a click,
    // which is why Board.qml lets go of the focus before it makes the move.
    void theFirstClickAfterAMenuDialogSelectsAPiece()
    {
        my_app->move ("e2", "e4");
        chooseFromMenu (QStringLiteral ("New Game"));
        auto* new_game = my_app->popupWithTitle (QStringLiteral ("New Game"));
        QTRY_VERIFY( new_game->property ("visible").toBool() );
        clickButton (QStringLiteral ("No"));
        QTRY_VERIFY( !new_game->property ("visible").toBool() );

        my_app->move ("d7", "d5");

        QVERIFY( my_app->boardPieceAt ("d5") == ColoredPiece::make (Color::Black, Piece::Pawn) );
    }

    void theFirstClickAfterADrawOfferSelectsAPiece()
    {
        repeatThePositionThreeTimes();
        auto* dialog = my_app->popupWithTitle (QStringLiteral ("Draw Offer"));
        QTRY_VERIFY( dialog->property ("visible").toBool() );
        clickButton (QStringLiteral ("No"));
        QTRY_VERIFY( !dialog->property ("visible").toBool() );

        my_app->move ("e2", "e4");

        QVERIFY( my_app->boardPieceAt ("e4") == ColoredPiece::make (Color::White, Piece::Pawn) );
    }

    void theEngineAnswersAMove()
    {
        my_app->changeGameSetting ("maxDepth", 1);
        my_app->changeGameSetting ("maxSearchTime", 1);
        my_app->changeGameSetting ("blackPlayer", wisdom::ui::Player::Computer);
        QSignalSpy engine_moved { &my_app->game_model, &GameModel::engineMoved };

        my_app->move ("e2", "e4");

        QVERIFY( engine_moved.wait (15000) );
        QVERIFY( my_app->game_model.qmlCurrentTurn() == wisdom::ui::Color::White );
        QTRY_VERIFY( my_app->piecesMatchTheBoard() );
        QTRY_VERIFY( my_app->showsText (QStringLiteral ("<b>White</b> to move")) );

        // And again, so the engine is shown to follow the game.
        my_app->move ("d2", "d4");

        QVERIFY( engine_moved.wait (15000) );
        QCOMPARE( engine_moved.count(), 2 );
        QTRY_VERIFY( my_app->piecesMatchTheBoard() );
    }

private:
    void openMenu()
    {
        auto tool_buttons = my_app->shownItemsOfClass ("QQuickToolButton");
        QCOMPARE( tool_buttons.size(), 1 );
        my_app->clickItem (tool_buttons[0]);
        QTRY_VERIFY( my_app->buttonWithText (QStringLiteral ("Quit")) != nullptr );
    }

    void clickButton (const QString& text)
    {
        QTRY_VERIFY( my_app->buttonWithText (text) != nullptr );
        my_app->clickItem (my_app->buttonWithText (text));
    }

    void chooseFromMenu (const QString& text)
    {
        openMenu();
        clickButton (text);
    }

    // In the order they are laid out, top to bottom.
    [[nodiscard]] auto
    radioButtons (const QString& text) const
        -> QList<QQuickItem*>
    {
        QList<QQuickItem*> result;
        for (auto* item : my_app->shownItemsOfClass ("QQuickRadioButton"))
        {
            if (item->property ("text").toString() == text)
                result << item;
        }
        std::sort (result.begin(), result.end(),
            [] (const QQuickItem* a, const QQuickItem* b)
            {
                return a->mapToScene (QPointF { 0, 0 }).y() < b->mapToScene (QPointF { 0, 0 }).y();
            });
        return result;
    }

    void repeatThePositionThreeTimes()
    {
        for (int i = 0; i < 2; i++)
        {
            my_app->move ("g1", "f3");
            my_app->move ("g8", "f6");
            my_app->move ("f3", "g1");
            my_app->move ("f6", "g8");
        }
    }

    std::unique_ptr<Application> my_app;
};

QTEST_MAIN( DialogsTest )

#include "dialogs_test.moc"
