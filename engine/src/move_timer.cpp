#include "move_timer.hpp"

namespace wisdom
{
    using chrono::steady_clock;

    constexpr int Num_Calls_Per_Timer_Check = 10000;   // number of iterations before checking

    auto MoveTimer::is_triggered () -> bool
    {
        if (!my_started)
            return false;

        if (my_triggered)
            return true;

        if (++my_check_calls % Num_Calls_Per_Timer_Check != 0)
            return false;

        if (my_periodic_function.has_value ())
        {
            (*my_periodic_function) (this);
            if (my_triggered)
                return true;
        }

        steady_clock::time_point next_check_time = steady_clock::now ();
        auto diff_time = next_check_time - my_last_check_time;

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
