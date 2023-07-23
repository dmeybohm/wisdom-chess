#include "move_timer.hpp"

namespace wisdom
{
    using chrono::steady_clock;

    static constexpr int Num_Iterations_Before_Checking = 10000;

    auto MoveTimer::isTriggered() -> bool
    {
        if (!my_started_time.has_value())
            return false;

        if (my_triggered || my_cancelled)
            return true;

        if (++my_check_calls % Num_Iterations_Before_Checking != 0)
            return false;

        if (my_periodic_function.has_value())
        {
            (*my_periodic_function) (this);
            if (my_triggered || my_cancelled)
                return true;
        }

        steady_clock::time_point check_time = steady_clock::now();
        auto diff_time = check_time - *my_started_time;

        if (diff_time >= my_seconds)
        {
            my_triggered = true;
            return true;
        }
        else
        {
            return false;
        }
    }
}
