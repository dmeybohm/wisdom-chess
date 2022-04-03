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

ChessEngine::ChessEngine(
        shared_ptr<Game> game, not_null<mutex*> gameMutex, QObject *parent)
    : QObject { parent },
     myGame { std::move(game) },
     myGameMutex { gameMutex }
{
}

void ChessEngine::init()
{
    if (myGame->is_computer_turn()) {
        findMove();
    }
}

void ChessEngine::opponentMoved()
{
    findMove();
}

void ChessEngine::findMove()
{
    std::lock_guard guard { *myGameMutex };
    Logger& output = make_standard_logger();

    auto who = myGame->get_current_turn();
    if (is_checkmated(myGame->get_board(), who)) {
        std::cout << to_string(color_invert(myGame->get_current_turn())) << " wins the game.\n";
        return;
    }

    //        if (myGame.get_history().is_third_repetition(myGame.get_board())) {
    //            input_state = offer_draw();
    //            continue;
    //        }

    if (History::is_fifty_move_repetition(myGame->get_board())) {
        std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
        return;
    }

    assert (myGame->is_computer_turn());
    qDebug() << "Searching for move";
    auto optionalMove = myGame->find_best_move(output);

    // TODO: we could have timed out or the thread was interrupted, and we should distinguish
    // between these two cases. If we couldn't find any move in the time, should select a move
    // at random, and otherwise exit.
    if (optionalMove.has_value()) {
        myGame->move(*optionalMove);
        emit engineMoved(*optionalMove, who);
    }
}
