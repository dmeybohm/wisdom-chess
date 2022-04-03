#ifndef GAMETHREADNOTIFIER_H
#define GAMETHREADNOTIFIER_H

#include "move_timer.hpp"

#include <QAtomicInt>

class GameThreadNotifier : public wisdom::PeriodicNotified
{
public:
    GameThreadNotifier();
    virtual ~GameThreadNotifier() = default;
    // PeriodicNotified interface

public:
    void notify(wisdom::MoveTimer *timer) override;

    void setCancelled()
    {
        ++myCancelled;
    }

    bool isCancelled()
    {
        int cancelled = myCancelled;
        return cancelled > 0;
    }

private:
    QAtomicInt myCancelled = 0;
};

#endif // GAMETHREADNOTIFIER_H
