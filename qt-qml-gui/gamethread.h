#ifndef GAMETHREAD_H
#define GAMETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "game.hpp"
#include "move.hpp"

//
// Represents the computer player.
//
class GameThread : public QThread
{
    Q_OBJECT
public:
    explicit GameThread(QObject *parent = nullptr);

    void run() override;

public slots:
    void humanMoved(wisdom::Move move);

signals:
    void computerMoved(wisdom::Move move);

private:
    wisdom::Game myGame;

    // When calling the slot, we want to signal the secondary thread to resume the loop.
    QMutex myGameMutex;
    QWaitCondition myWaitForHumanMovedCondition;
};

#endif // GAMETHREAD_H
