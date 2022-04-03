#ifndef CHESSENGINENOTIFIER_H
#define CHESSENGINENOTIFIER_H

#include "move_timer.hpp"

#include <QAtomicInt>

class ChessEngineNotifier : public wisdom::PeriodicNotified
{
public:
    ChessEngineNotifier() = default;
    virtual ~ChessEngineNotifier() = default;

    // PeriodicNotified interface
public:
    void notify(gsl::not_null<wisdom::MoveTimer*> timer) override;
};

#endif // CHESSENGINENOTIFIER_H
