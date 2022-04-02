#include <QDebug>

#include "gamethread.h"
#include "check.hpp"
#include "logger.hpp"

using namespace wisdom;

GameThread::GameThread(QObject* parent)
        : QThread { parent },
         myGame { Color::White, Color::Black }
{
}

void GameThread::run()
{
    Logger& output = make_standard_logger();

    while (true) {
        myGameMutex.lock();

        if (is_checkmated(myGame.get_board(), myGame.get_current_turn())) {
            std::cout << to_string(color_invert(myGame.get_current_turn())) << " wins the game.\n";
            myGameMutex.unlock();
            return;
        }

//        if (myGame.get_history().is_third_repetition(myGame.get_board())) {
//            input_state = offer_draw();
//            continue;
//        }

        if (History::is_fifty_move_repetition(myGame.get_board())) {
            std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
            myGameMutex.unlock();
            break;
        }

        if (myGame.is_computer_turn()) {
            qDebug() << "Searching for move";
            auto optionalMove = myGame.find_best_move(output);
            if (!optionalMove.has_value()) {
                std::cout << "\nCouldn't find move!\n";
                // todo: could be timed out. check legal moves at random and select one
                myGameMutex.unlock();
                break;
            }
            myGame.move(*optionalMove);
            emit computerMoved(*optionalMove);
        } else {
            // Wait for human move.
            qDebug() << "Waiting for human move...";
            myWaitForHumanMovedCondition.wait(&myGameMutex);

        }
        myGameMutex.unlock();
    }

}

void GameThread::humanMoved(Move move)
{
    myGameMutex.lock();
    if (this != QThread::currentThread()) {
        qDebug() << "*** Current thread != this!";
    } else {
        qDebug() << "*** Current thread == this";
    }
    myGame.move(move);

    // Let the computer know we've moved.
    myWaitForHumanMovedCondition.wakeOne();
    myGameMutex.unlock();
}
