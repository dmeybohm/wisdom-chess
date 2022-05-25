#include "chessgame.hpp"
#include "fen_parser.hpp"
#include "move_timer.hpp"

#include <QDebug>

using wisdom::Game;
using wisdom::Move;
using wisdom::FenParser;
using wisdom::MoveTimer;
using wisdom::Color;
using wisdom::Piece;
using std::unique_ptr;
using std::string;
using std::atomic;
using std::optional;
using std::pair;
using std::make_unique;
using gsl::not_null;

auto ChessGame::fromPlayers(
        wisdom::Player whitePlayer,
        wisdom::Player blackPlayer,
        Config config
)
    -> unique_ptr<ChessGame>
{
    return fromEngine(std::move(make_unique<Game>(whitePlayer, blackPlayer)), config);
}

auto ChessGame::fromFen(const string &input, Config config) -> unique_ptr<ChessGame>
{
    FenParser parser { input };
    auto game = parser.build ();
    return fromEngine(std::make_unique<Game>(std::move(game)), config);
}

auto ChessGame::fromEngine(std::unique_ptr<wisdom::Game> game, Config config) ->
    unique_ptr<ChessGame>
{
    return make_unique<ChessGame>(std::move(game), config);
}

auto ChessGame::clone() const ->
    std::unique_ptr<ChessGame>
{
    // Copy current game state to FEN and send on to the chess engine thread:
    auto currentGame = this->engine();
    auto whitePlayer = currentGame->get_player(wisdom::Color::White);
    auto blackPlayer = currentGame->get_player(wisdom::Color::Black);

    auto fen = currentGame->get_board().to_fen_string(currentGame->get_current_turn());
    auto newGame = ChessGame::fromFen(fen, myConfig);
    newGame->engine()->set_white_player(whitePlayer);
    newGame->engine()->set_black_player(blackPlayer);
    return newGame;
}

auto ChessGame::isLegalMove(Move selectedMove) -> bool
{
    auto game = this->engine();
    auto selectedMoveStr = to_string(selectedMove);
    qDebug() << "Selected move: " << QString(selectedMoveStr.c_str());

    // If it's not the human's turn, move is illegal.
    if (game->get_current_player() != wisdom::Player::Human) {
        return false;
    }

    auto who = game->get_current_turn();
    auto generator = game->get_move_generator();
    auto legalMoves = generator->generate_legal_moves(game->get_board(), who);
    auto legalMovesStr = to_string(legalMoves);
    qDebug() << QString(legalMovesStr.c_str());

    for (auto legalMove : legalMoves) {
        if (legalMove == selectedMove) {
            return true;
        }
    }
    return false;
}

void ChessGame::setConfig(Config config)
{
    auto gameState = this->engine();
    gameState->set_max_depth(config.maxDepth.internalDepth());
    gameState->set_search_timeout(config.maxTime);
    myConfig = config;
}

void ChessGame::setPlayers(wisdom::Player whitePlayer, wisdom::Player blackPlayer)
{
   auto gameState = this->engine();
   gameState->set_white_player(whitePlayer);
   gameState->set_black_player(blackPlayer);
}

void ChessGame::setupNotify(atomic<int>* gameId)
{
    auto engine = this->engine();
    auto initialGameId = gameId->load();

    engine->set_periodic_function([initialGameId, gameId](not_null<MoveTimer*> moveTimer) {
        // This runs in the ChessEngine thread.
        // Check if the gameId we passed in originally has changed - if so,
        // the game is over.
        if (initialGameId != gameId->load()) {
            qDebug() << "Setting timeout to break the loop.";
            moveTimer->set_triggered(true);
        }
    });
}

auto ChessGame::moveFromCoordinates(int srcRow, int srcColumn,
                                    int dstRow, int dstColumn, optional<Piece> promoted)
    -> pair<optional<Move>, Color>
{
    auto engine = this->engine();
    auto src = wisdom::make_coord(srcRow, srcColumn);
    auto dst = wisdom::make_coord(dstRow, dstColumn);

    auto who = engine->get_current_turn();
    qDebug() << "Mapping coordinates for " << srcRow << ":" << srcColumn << " -> "
         << dstRow << ":" << dstColumn;

    return { engine->map_coordinates_to_move(src, dst, promoted), who };
}
