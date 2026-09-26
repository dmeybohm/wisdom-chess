#include "wisdom-chess/engine/move_timer.hpp"

namespace wisdom
{
    auto MoveTimer::isTriggered() -> bool
    {
        if (!my_timer_state.started_time.has_value())
            return false;

        if (my_timer_state.triggered)
            return true;

        if ((++my_timer_state.check_calls & (Calls_Between_Clock_Checks - 1)) != 0)
            return false;

        if (my_periodic_function.has_value())
        {
            (*my_periodic_function) (this);
            if (my_timer_state.triggered)
                return true;
        }

        auto elapsed = chrono::steady_clock::now() - *my_timer_state.started_time;
        if (elapsed >= my_time_limit)
            my_timer_state.triggered = true;

        return my_timer_state.triggered;
    }
}
