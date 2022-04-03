#include <QThread>
#include <QDebug>

#include "chessenginenotifier.h"

void ChessEngineNotifier::notify(gsl::not_null<wisdom::MoveTimer*> timer)
{
    auto* currentThread = QThread::currentThread();

    if (currentThread->isInterruptionRequested()) {
        qDebug() << "Setting timeout to break the loop.";
        timer->set_triggered(true);
    }
}
