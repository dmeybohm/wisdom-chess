#include <QDebug>

#include "gamethread.h"
#include "check.hpp"
#include "logger.hpp"

using namespace wisdom;

GameThread::GameThread(QObject* parent)
        : QThread { parent },
         myGame { Color::White, Color::Black },
         myEventLoop { nullptr }
{
}

void GameThread::run()
{
    myEventLoop = new QEventLoop { this };
    myEventLoop->moveToThread(this);
    Logger& output = make_standard_logger();

    while (true) {
        if (is_checkmated(myGame.get_board(), myGame.get_current_turn())) {
            std::cout << to_string(color_invert(myGame.get_current_turn())) << " wins the game.\n";
            return;
        }

//        if (myGame.get_history().is_third_repetition(myGame.get_board())) {
//            input_state = offer_draw();
//            continue;
//        }

        if (History::is_fifty_move_repetition(myGame.get_board())) {
            std::cout << "Fifty moves without a capture or pawn move. It's a draw!\n";
            break;
        }

        if (myGame.is_computer_turn()) {
            qDebug() << "Searching for move";
            auto optionalMove = myGame.find_best_move(output);
            if (!optionalMove.has_value()) {
                std::cout << "\nCouldn't find move!\n";
                // todo: could be timed out. check legal moves at random and select one
                break;
            }
            emit computerMoved(*optionalMove);
        } else {
            // process events:
            qDebug() << "Waiting for human move...";
            myEventLoop->exec();
            qDebug() << "Event loop exited...";
        }
    }
}

void GameThread::humanMoved(Move move)
{
    myGame.move(move);

    // Let the computer know we've moved.
    myEventLoop->quit();
}
