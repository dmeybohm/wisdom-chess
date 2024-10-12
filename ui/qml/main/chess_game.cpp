#include <QDebug>

#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/generate.hpp"

#include "wisdom-chess/qml/chess_game.hpp"
#include "wisdom-chess/qml/game_settings.hpp"

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

auto 
ChessGame::fromPlayers (
    wisdom::Player whitePlayer, 
    wisdom::Player blackPlayer, 
    const Config& config
) 
    -> unique_ptr<ChessGame>
{
    return fromEngine (std::move (make_unique<Game> (whitePlayer, blackPlayer)), config);
}

auto 
ChessGame::fromFen (
    const string& input, 
    const Config& config
) 
    -> unique_ptr<ChessGame>
{
    FenParser parser { input };
    auto game = parser.build();
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

auto 
ChessGame::isLegalMove (Move selectedMove) const 
    -> bool
{
    auto game = this->state();
    auto selectedMoveStr = asString (selectedMove);

    // If it's not the human's turn, move is illegal.
    if (game->getCurrentPlayer() != wisdom::Player::Human)
    {
        return false;
    }

    auto who = game->getCurrentTurn();
    auto legalMoves = generateLegalMoves (game->getBoard(), who);

    return std::any_of (legalMoves.cbegin(), legalMoves.cend(),
                        [selectedMove] (const auto& move)
                        {
                            return move == selectedMove;
                        });
}

void ChessGame::setConfig (const Config& config)
{
    auto gameState = this->state();
    gameState->setMaxDepth (config.maxDepth.internalDepth());
    gameState->setSearchTimeout (config.maxTime);
    gameState->setPlayers (config.players);
    my_config = config;
}

auto 
ChessGame::setPlayers (
    wisdom::Player whitePlayer,
    wisdom::Player blackPlayer
) 
    -> void
    // NOLINT(readability-make-member-function-const)
{
    auto gameState = this->state();
    gameState->setWhitePlayer (whitePlayer);
    gameState->setBlackPlayer (blackPlayer);
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

auto 
ChessGame::Config::fromGameSettings (
    const GameSettings& gameSettings
) 
    -> ChessGame::Config
{
    return ChessGame::Config {
        .players = { mapPlayer (gameSettings.whitePlayer()),
                     mapPlayer (gameSettings.blackPlayer()) },
        .maxDepth = MaxDepth { gameSettings.maxDepth() },
        .maxTime = std::chrono::seconds { gameSettings.maxSearchTime() }
    };
}
