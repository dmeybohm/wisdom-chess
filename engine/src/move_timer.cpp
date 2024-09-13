#include "move_timer.hpp"

#include <cmath>
#include <atomic>

namespace wisdom
{
    using chrono::steady_clock;

    std::atomic<int> TimingAdjustment::saved_iterations {};

    auto MoveTimer::isTriggered() -> bool
    {
        if (!my_timer_state.started_time.has_value())
            return false;

        if (my_timer_state.triggered || my_timer_state.cancelled)
            return true;

        if (++my_timer_state.check_calls % my_timing_adjustment.getIterations() != 0)
            return false;

        if (my_periodic_function.has_value())
        {
            (*my_periodic_function) (this);
            if (my_timer_state.triggered || my_timer_state.cancelled)
                return true;
        }

        steady_clock::time_point check_time = steady_clock::now();
        auto diff_time = check_time - *my_timer_state.started_time;

        // adjust the next iteration if last check took too long / too short:
        if (my_timer_state.last_check_time.has_value())
        {
            auto last_time = *my_timer_state.last_check_time;
            auto check_time_diff = check_time - last_time;

            auto& timing = my_timing_adjustment;
            if (
                check_time_diff < Lower_Bound_Timer_Check &&
                timing.getIterations() < Max_Iterations_Before_Checking
            ) {
                timing.setIterations (narrow<int> (std::floor (timing.getIterations() * 1.5)));
                if (timing.getIterations() > Max_Iterations_Before_Checking)
                    timing.setIterations (Max_Iterations_Before_Checking);
            } else if (
                check_time_diff > Upper_Bound_Timer_Check &&
                timing.getIterations() > Min_Iterations_Before_Checking
            ) {
                timing.setIterations (timing.getIterations() / 2);
                if (timing.getIterations() < Min_Iterations_Before_Checking)
                    timing.setIterations (Min_Iterations_Before_Checking);
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

    auto TimingAdjustment::create() -> TimingAdjustment
    {
        auto iterations = std::clamp (
            TimingAdjustment::saved_iterations.load(),
            Min_Iterations_Before_Checking,
            Max_Iterations_Before_Checking
        );
        return TimingAdjustment (iterations);
    }

    void TimingAdjustment::setSavedIterations (int iterations)
    {
        TimingAdjustment::saved_iterations.store (iterations);
    }
}
