#include <QTest>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/ui/qml/main/chess_game.hpp"
#include "wisdom-chess/ui/qml/main/game_settings.hpp"

using wisdom::Color;
using wisdom::moveParse;
using wisdom::Piece;
using wisdom::Player;

namespace
{
    auto
    makeConfig (Player white = Player::Human, Player black = Player::Human)
        -> ChessGame::Config
    {
        return ChessGame::Config {
            .players = { white, black },
            .maxDepth = MaxDepth { 3 },
            .maxTime = std::chrono::seconds { 7 },
        };
    }

    auto
    fenOf (const ChessGame& game)
        -> std::string
    {
        auto state = game.state();
        return state->getBoard().toFenString (state->getCurrentTurn());
    }

    template <typename Value>
    void writeProperty (GameSettings& settings, const char* name, Value value)
    {
        const auto& meta_object = GameSettings::staticMetaObject;
        auto property = meta_object.property (meta_object.indexOfProperty (name));
        QVERIFY( property.writeOnGadget (&settings, QVariant::fromValue (value)) );
    }
}

class ChessGameTest : public QObject
{
    Q_OBJECT

private slots:
    void maxDepthCountsFullMoves()
    {
        MaxDepth depth { 4 };

        QCOMPARE( depth.userDepth(), 4 );
        QCOMPARE( depth.internalDepth(), 8 );
    }

    void maxDepthMustBePositive()
    {
        QVERIFY_THROWS_EXCEPTION( wisdom::Error, MaxDepth { 0 } );
        QVERIFY_THROWS_EXCEPTION( wisdom::Error, MaxDepth { -1 } );
    }

    void configFromDefaultGameSettings()
    {
        GameSettings settings;

        auto config = ChessGame::Config::fromGameSettings (settings);

        QVERIFY( config.players[0] == Player::Human );
        QVERIFY( config.players[1] == Player::ChessEngine );
        QCOMPARE( config.maxDepth.userDepth(), wisdom::Default_Max_Depth / 2 );
        QCOMPARE( config.maxDepth.internalDepth(), wisdom::Default_Max_Depth );
        QCOMPARE( config.maxTime.count(), wisdom::Default_Max_Search_Seconds );
        QCOMPARE( config.debugLogging, false );
    }

    void configFromChangedGameSettings()
    {
        GameSettings settings;
        writeProperty (settings, "whitePlayer", wisdom::ui::Player::Computer);
        writeProperty (settings, "blackPlayer", wisdom::ui::Player::Human);
        writeProperty (settings, "maxDepth", 2);
        writeProperty (settings, "maxSearchTime", 9);
        writeProperty (settings, "debugLogging", true);

        auto config = ChessGame::Config::fromGameSettings (settings);

        QVERIFY( config.players[0] == Player::ChessEngine );
        QVERIFY( config.players[1] == Player::Human );
        QCOMPARE( config.maxDepth.internalDepth(), 4 );
        QCOMPARE( config.maxTime.count(), 9 );
        QCOMPARE( config.debugLogging, true );
    }

    void aGameAppliesItsConfigToTheEngine()
    {
        auto game = ChessGame::fromPlayers (Player::Human, Player::ChessEngine, makeConfig());
        auto state = game->state();

        QCOMPARE( state->getMaxDepth(), 6 );
        QCOMPARE( state->getSearchTimeout().count(), 7 );
        QCOMPARE( game->config().maxDepth.userDepth(), 3 );
    }

    // The config carries players too. The ones asked for by name win, and
    // the stored config agrees with the game afterwards.
    void fromPlayersUsesThePlayersItIsGiven()
    {
        auto game = ChessGame::fromPlayers (
            Player::ChessEngine, Player::Human, makeConfig (Player::Human, Player::Human)
        );

        QVERIFY( game->state()->getPlayer (Color::White) == Player::ChessEngine );
        QVERIFY( game->state()->getPlayer (Color::Black) == Player::Human );
        QVERIFY( game->config().players[0] == Player::ChessEngine );
        QVERIFY( game->config().players[1] == Player::Human );
    }

    void fromFenTakesThePlayersFromTheConfig()
    {
        auto game = ChessGame::fromFen (
            "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", makeConfig (Player::ChessEngine, Player::Human)
        );

        QVERIFY( game->state()->getPlayer (Color::White) == Player::ChessEngine );
        QVERIFY( game->state()->getPlayer (Color::Black) == Player::Human );
    }

    void setConfigReplacesTheSettings()
    {
        auto game = ChessGame::fromPlayers (Player::Human, Player::Human, makeConfig());

        game->setConfig (ChessGame::Config {
            .players = { Player::ChessEngine, Player::Human },
            .maxDepth = MaxDepth { 1 },
            .maxTime = std::chrono::seconds { 2 },
            .debugLogging = true,
        });

        auto state = game->state();
        QCOMPARE( state->getMaxDepth(), 2 );
        QCOMPARE( state->getSearchTimeout().count(), 2 );
        QVERIFY( state->getPlayer (Color::White) == Player::ChessEngine );
        QCOMPARE( game->config().debugLogging, true );
    }

    void setPlayers()
    {
        auto game = ChessGame::fromPlayers (Player::Human, Player::Human, makeConfig());

        game->setPlayers (Player::ChessEngine, Player::Human);

        QVERIFY( game->state()->getPlayer (Color::White) == Player::ChessEngine );
        QVERIFY( game->state()->getPlayer (Color::Black) == Player::Human );
        QVERIFY( game->config().players[0] == Player::ChessEngine );
        QVERIFY( game->config().players[1] == Player::Human );
    }

    void fromFenLoadsThePositionAndTheSideToMove()
    {
        auto game = ChessGame::fromFen ("4k3/8/8/8/8/8/8/R3K3 b Q - 3 20", makeConfig());

        QVERIFY( game->state()->getCurrentTurn() == Color::Black );
        QCOMPARE( fenOf (*game), std::string { "4k3/8/8/8/8/8/8/R3K3 b Q - 3 20" } );
    }

    void fromFenRejectsNonsense()
    {
        QVERIFY_THROWS_EXCEPTION( wisdom::Error, ChessGame::fromFen ("not a fen", makeConfig()) );
    }

    void aCloneHasThePositionPlayersAndConfig()
    {
        auto game = ChessGame::fromPlayers (Player::Human, Player::ChessEngine, makeConfig());
        game->state()->move (moveParse ("e2 e4", Color::White));
        game->setPlayers (Player::ChessEngine, Player::Human);

        auto clone = game->clone();

        QCOMPARE( fenOf (*clone), fenOf (*game) );
        QVERIFY( clone->state()->getPlayers() == game->state()->getPlayers() );
        QVERIFY( clone->config().players == clone->state()->getPlayers() );
        QCOMPARE( clone->state()->getMaxDepth(), game->state()->getMaxDepth() );
        QCOMPARE( clone->state()->getSearchTimeout().count(), 7 );

        // Documented on clone(): the moves played so far are not copied.
        QVERIFY( clone->state()->getHistory().getMoveHistory().empty() );
    }

    void aCloneIsIndependent()
    {
        auto game = ChessGame::fromPlayers (Player::Human, Player::Human, makeConfig());
        auto clone = game->clone();

        clone->state()->move (moveParse ("e2 e4", Color::White));

        QVERIFY( fenOf (*clone) != fenOf (*game) );
        QVERIFY( game->state()->getCurrentTurn() == Color::White );
    }

    void moveFromCoordinates()
    {
        auto game = ChessGame::fromPlayers (Player::Human, Player::Human, makeConfig());
        auto src = wisdom::coordParse ("e2");
        auto dst = wisdom::coordParse ("e4");

        auto [move, who] = game->moveFromCoordinates (
            src.row<int>(), src.column<int>(), dst.row<int>(), dst.column<int>(), std::nullopt
        );

        QVERIFY( who == Color::White );
        QVERIFY( move.has_value() );
        QVERIFY( *move == moveParse ("e2 e4", Color::White) );
    }

    void moveFromCoordinatesRecognizesSpecialMoves()
    {
        auto map = [] (const ChessGame& game, const char* src_text, const char* dst_text,
                       std::optional<Piece> promoted = std::nullopt)
        {
            auto src = wisdom::coordParse (src_text);
            auto dst = wisdom::coordParse (dst_text);
            return game.moveFromCoordinates (
                src.row<int>(), src.column<int>(), dst.row<int>(), dst.column<int>(), promoted
            ).first;
        };

        auto castling = ChessGame::fromFen ("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", makeConfig());
        auto promoting = ChessGame::fromFen ("1n2k3/P7/8/8/8/8/8/4K3 w - - 0 1", makeConfig());
        auto en_passant = ChessGame::fromFen ("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", makeConfig());

        QVERIFY( map (*castling, "e1", "g1") == moveParse ("o-o", Color::White) );
        QVERIFY( map (*castling, "e1", "c1") == moveParse ("o-o-o", Color::White) );
        QVERIFY( map (*promoting, "a7", "a8", Piece::Rook) == moveParse ("a7 a8(R)", Color::White) );
        QVERIFY( map (*promoting, "a7", "b8", Piece::Queen) == moveParse ("a7xb8(Q)", Color::White) );
        QVERIFY( map (*en_passant, "e5", "d6") == moveParse ("e5 d6 ep", Color::White) );
        QVERIFY( !map (*castling, "c3", "c4").has_value() );
    }
};

QTEST_GUILESS_MAIN( ChessGameTest )

#include "chess_game_test.moc"
