#include <QDebug>

#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/board.hpp"

#include "wisdom-chess/ui/qml/main/chess_game.hpp"
#include "wisdom-chess/ui/qml/main/game_settings.hpp"

using gsl::not_null;
using std::atomic;
using std::make_unique;
using std::optional;
using std::pair;
using std::string;
using std::unique_ptr;
using wisdom::Color;
using wisdom::FenParser;
using wisdom::Game;
using wisdom::Move;
using wisdom::MoveTimer;
using wisdom::Piece;

namespace wisdom::ui::qml
{
    // The engine's types, not the wisdom::ui mirrors QML sees.
    using wisdom::Color;
    using wisdom::Player;

    auto 
    ChessGame::fromPlayers (
        wisdom::Player whitePlayer, 
        wisdom::Player blackPlayer, 
        const Config& config
    ) 
        -> unique_ptr<ChessGame>
    {
        auto config_with_players = config;
        config_with_players.players = { whitePlayer, blackPlayer };

        return fromEngine (
            make_unique<Game> (Game::createGame (whitePlayer, blackPlayer)),
            config_with_players
        );
    }

    auto 
    ChessGame::fromFen (
        const string& input, 
        const Config& config
    ) 
        -> unique_ptr<ChessGame>
    {
        auto game = Game::createGameFromFen (input);
        return fromEngine (std::make_unique<Game> (std::move (game)), config);
    }

    auto 
    ChessGame::fromEngine (
        std::unique_ptr<wisdom::Game> game, 
        const Config& config
    )
        -> unique_ptr<ChessGame>
    {
        return make_unique<ChessGame> (std::move (game), config);
    }

    auto 
    ChessGame::clone() const 
        -> std::unique_ptr<ChessGame>
    {
        // Copy current game state to FEN and send on to the chess engine thread:
        auto currentGame = this->state();
        auto players = currentGame->getPlayers();
        auto newConfig = my_config;

        auto fen = currentGame->getBoard().toFenString (currentGame->getCurrentTurn());
        auto newGame = ChessGame::fromFen (fen, newConfig);
        newGame->state()->setPlayers (players);
        return newGame;
    }

    void ChessGame::setConfig (const Config& config)
    {
        config.applyTo (this->state());
        my_config = config;
    }

    void
    ChessGame::setPlayers (
        wisdom::Player whitePlayer,
        wisdom::Player blackPlayer
    ) { // NOLINT(readability-make-member-function-const)
        const wisdom::Players players { whitePlayer, blackPlayer };
        this->state()->setPlayers (players);
        my_config.players = players;
    }

    auto 
    ChessGame::moveFromCoordinates (
        int srcRow, 
        int srcColumn, 
        int dstRow, 
        int dstColumn,
        optional<Piece> promoted
    ) const 
        -> pair<optional<Move>, Color>
    {
        auto engine = this->state();
        auto src = wisdom::makeCoord (srcRow, srcColumn);
        auto dst = wisdom::makeCoord (dstRow, dstColumn);

        auto who = engine->getCurrentTurn();

        return { engine->mapCoordinatesToMove (src, dst, promoted), who };
    }

    void ChessGame::setPeriodicFunction (const MoveTimer::PeriodicFunction& func)
    {
        auto gameState = this->state();
        gameState->setPeriodicFunction (func);
    }
}
