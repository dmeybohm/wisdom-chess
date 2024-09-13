#include "move_timer.hpp"

#include <cmath>

namespace wisdom
{
    using chrono::steady_clock;

    auto MoveTimer::isTriggered() -> bool
    {
        if (!my_timer_state.started_time.has_value())
            return false;

        if (my_timer_state.triggered || my_timer_state.cancelled)
            return true;

        if (++my_timer_state.check_calls % my_timing_adjustment->current_iterations != 0)
            return false;

        if (my_periodic_function.has_value())
        {
            (*my_periodic_function) (this);
            if (my_timer_state.triggered || my_timer_state.cancelled)
                return true;
        }

        steady_clock::time_point check_time = steady_clock::now();
        auto diff_time = check_time - *my_timer_state.started_time;

        // adjust the next iteration if less than 250ms:
        if (my_timer_state.last_check_time.has_value())
        {
            auto last_time = *my_timer_state.last_check_time;
            auto check_time_diff = check_time - last_time;

            auto& timing = *my_timing_adjustment;
            if (
                check_time_diff < Lower_Bound_Timer_Check &&
                timing.current_iterations < Max_Iterations_Before_Checking
            ) {
                timing.current_iterations = narrow<int> (std::floor (timing.current_iterations * 1.5));
                if (timing.current_iterations > Max_Iterations_Before_Checking)
                    timing.current_iterations = Max_Iterations_Before_Checking;

            } else if (
                check_time_diff > Upper_Bound_Timer_Check &&
                timing.current_iterations > Min_Iterations_Before_Checking
            ) {
                timing.current_iterations /= 2;
                if (timing.current_iterations < Min_Iterations_Before_Checking)
                    timing.current_iterations = Min_Iterations_Before_Checking;
            }
        }

        my_timer_state.last_check_time = check_time;

        if (diff_time >= my_seconds)
        {
            my_timer_state.triggered = true;
            return true;
        }
        else
        {
            return false;
        }
    }
}
