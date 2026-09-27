#include <QDebug>

#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/board.hpp"

#include "wisdom-chess/ui/qml/main/chess_game.hpp"
#include "wisdom-chess/ui/qml/main/game_settings.hpp"

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
        wisdom::Player white_player,
        wisdom::Player black_player,
        const Config& config
    )
        -> unique_ptr<ChessGame>
    {
        auto config_with_players = config;
        config_with_players.players = { white_player, black_player };

        return fromEngine (
            make_unique<Game> (Game::createGame (white_player, black_player)),
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
        auto current_game = this->state();
        auto players = current_game->getPlayers();
        auto new_config = my_config;

        auto fen = current_game->getBoard().toFenString (current_game->getCurrentTurn());
        auto new_game = ChessGame::fromFen (fen, new_config);
        new_game->state()->setPlayers (players);
        return new_game;
    }

    void ChessGame::setConfig (const Config& config)
    {
        config.applyTo (this->state());
        my_config = config;
    }

    void
    ChessGame::setPlayers (
        wisdom::Player white_player,
        wisdom::Player black_player
    ) { // NOLINT(readability-make-member-function-const)
        const wisdom::Players players { white_player, black_player };
        this->state()->setPlayers (players);
        my_config.players = players;
    }

    auto
    ChessGame::moveFromCoordinates (
        int src_row,
        int src_column,
        int dst_row,
        int dst_column,
        optional<Piece> promoted
    ) const
        -> pair<optional<Move>, Color>
    {
        auto engine = this->state();
        auto src = wisdom::makeCoord (src_row, src_column);
        auto dst = wisdom::makeCoord (dst_row, dst_column);

        auto who = engine->getCurrentTurn();

        return { engine->mapCoordinatesToMove (src, dst, promoted), who };
    }

    void ChessGame::setPeriodicFunction (const MoveTimer::PeriodicFunction& func)
    {
        auto game_state = this->state();
        game_state->setPeriodicFunction (func);
    }
}
