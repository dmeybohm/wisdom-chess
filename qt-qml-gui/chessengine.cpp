#include <QThread>
#include <QDebug>
#include <mutex>

#include "check.hpp"
#include "chessengine.hpp"
#include "logger.hpp"

using namespace wisdom;
using std::shared_ptr;
using std::optional;
using gsl::not_null;

ChessEngine::ChessEngine(shared_ptr<ChessGame> game, int gameId, QObject *parent)
    : QObject { parent }
    , myGame { std::move(game) }
    , myGameId { gameId }
{
}

void ChessEngine::init()
{
    findMove();
}

void ChessEngine::opponentMoved(Move move, Color who)
{
    QThread::currentThread()->usleep(500);
    auto game = myGame->engine();
    game->move(move);
    findMove();
}

void ChessEngine::receiveEngineMoved(wisdom::Move move, wisdom::Color who,
                                     int gameId)
{
    if (gameId == this->myGameId) {
        // Do another move if the engine is hooked up to itself:
        init();
    }
}

void ChessEngine::findMove()
{
    auto game = myGame->engine();
    Logger& output = make_standard_logger();

    auto player = game->get_current_player();
    if (player != Player::ChessEngine) {
        return;
    }

    auto who = game->get_current_turn();
    auto board = game->get_board();
    auto generator = game->get_move_generator();
    if (is_checkmated(board, who, *generator)) {
        auto who = to_string(color_invert(game->get_current_turn()));
        qDebug() << who.c_str() << " wins the game.\n";
        emit noMovesAvailable();
        return;
    }

    if (History::is_fifty_move_repetition(game->get_board())) {
        qDebug() << "Fifty moves without a capture or pawn move. It's a draw!\n";
        emit noMovesAvailable();
        return;
    }

    qDebug() << "Searching for move";
    auto optionalMove = game->find_best_move(output);

    // TODO: we could have timed out or the thread was interrupted, and we should distinguish
    // between these two cases. If we couldn't find any move in the time, should select a move
    // at random, and otherwise exit.
    if (optionalMove.has_value()) {
        game->move(*optionalMove);
        emit engineMoved(*optionalMove, who, myGameId);
    } else {
        emit noMovesAvailable();
    }
}

void ChessEngine::drawProposed()
{
    emit drawProposalResponse(true);
}

void ChessEngine::reloadGame(shared_ptr<ChessGame> newGame, int newGameId)
{
    myGame = std::move(newGame);
    myGameId = newGameId;

    // Possibly resume searching for the next move:
    init();
}
