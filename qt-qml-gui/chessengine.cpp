#include <QThread>
#include <QDebug>
#include <mutex>

#include "chessenginenotifier.h"
#include "chessengine.h"
#include "check.hpp"
#include "logger.hpp"

using namespace wisdom;
using namespace std;
using namespace gsl;

ChessEngine::ChessEngine(shared_ptr<ChessGame> game, QObject *parent)
    : QObject { parent },
     myGame { std::move(game) }
{
}

void ChessEngine::init()
{
    bool isActive = [this]{
        auto lockedGame = myGame->access();
        return lockedGame->get_current_player() == Player::ChessEngine;
    }();
    if (isActive) {
        opponentMoved();
    }
}

void ChessEngine::opponentMoved()
{
    findMove();
    QThread::currentThread()->sleep(1);
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
    if (is_checkmated(game->get_board(), who)) {
        std::cout << to_string(color_invert(game->get_current_turn())) << " wins the game.\n";
        return;
    }

    //        if (myGame.get_history().is_third_repetition(myGame.get_board())) {
    //            input_state = offer_draw();
    //            continue;
    //        }

    if (History::is_fifty_move_repetition(game->get_board())) {
        std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
        return;
    }

    qDebug() << "Searching for move";
    auto optionalMove = game->find_best_move(output);

    // TODO: we could have timed out or the thread was interrupted, and we should distinguish
    // between these two cases. If we couldn't find any move in the time, should select a move
    // at random, and otherwise exit.
    if (optionalMove.has_value()) {
        game->move(*optionalMove);
        emit engineMoved(*optionalMove, who);
    }
}
