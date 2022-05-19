#include <QThread>
#include <QDebug>
#include <mutex>

#include "check.hpp"
#include "chessengine.hpp"
#include "logger.hpp"

using namespace wisdom;
using namespace std;
using namespace gsl;

int ChessEngine::lastEngineId;

ChessEngine::ChessEngine(std::unique_ptr<ChessGame> game, QObject *parent)
    : QObject { parent }
    , myGame { std::move(game) }
    , myEngineId { lastEngineId++ }
{
}

auto ChessEngine::engineId() -> int
{
    return myEngineId;
}

void ChessEngine::init()
{
    auto game = myGame->access();
    auto isActive = game->get_current_player() == Player::ChessEngine;
    if (isActive) {
        findMove();
    }
}

void ChessEngine::opponentMoved(Move move, Color who)
{
    QThread::currentThread()->usleep(500);
    auto game = myGame->access();
    game->move(move);
    findMove();
}

void ChessEngine::receiveEngineMoved(wisdom::Move move, wisdom::Color who,
                                     int engineId)
{
    if (engineId == this->myEngineId) {
        // Do another move if the engine is hooked up to itself:
        init();
    }
}

void ChessEngine::findMove()
{
    auto game = myGame->access();
    Logger& output = make_standard_logger();

    auto player = game->get_current_player();
    if (player != Player::ChessEngine) {
        return;
    }

    auto who = game->get_current_turn();
    auto board = game->get_board();
    auto generator = game->get_move_generator();
    if (is_checkmated(board, who, *generator)) {
        std::cout << to_string(color_invert(game->get_current_turn())) << " wins the game.\n";
        emit noMovesAvailable();
        return;
    }

    if (History::is_fifty_move_repetition(game->get_board())) {
        std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
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
        emit engineMoved(*optionalMove, who, myEngineId);
    } else {
        emit noMovesAvailable();
    }
}

void ChessEngine::drawProposed()
{
    emit drawProposalResponse(true);
}
