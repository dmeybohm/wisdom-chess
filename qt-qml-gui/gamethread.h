#ifndef GAMETHREAD_H
#define GAMETHREAD_H

#include <QThread>
#include <QEventLoop>

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
    QEventLoop* myEventLoop;
};

#endif // GAMETHREAD_H
