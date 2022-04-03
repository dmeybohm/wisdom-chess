#include <QDebug>

#include "gamethreadnotifier.h"

GameThreadNotifier::GameThreadNotifier()
{

}

void GameThreadNotifier::notify(wisdom::MoveTimer* timer)
{
    if (isCancelled()) {
        qDebug() << "Setting timeout to break the loop.";
        timer->set_triggered(true);
    }
}
