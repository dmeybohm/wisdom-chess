#ifndef GAMETHREAD_H
#define GAMETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "game.hpp"
#include "move.hpp"
#include "move_timer.hpp"

class GameThreadNotifier;
//
// Represents the computer player.
//
class GameThread : public QThread
{
    Q_OBJECT
public:
    GameThread(gsl::not_null<GameThreadNotifier*> notifier, QObject *parent = nullptr);

    void run() override;

public slots:
    void prepareToExit();
    void humanMoved(wisdom::Move move);

signals:
    void computerMoved(wisdom::Move move);

private:
    wisdom::Game myGame;

    // When calling the slot, we want to signal the secondary thread to resume the loop.
    QMutex myGameMutex;
    QWaitCondition myWaitForHumanMovedCondition;
    gsl::not_null<GameThreadNotifier*> myNotifier;
};

#endif // GAMETHREAD_H
