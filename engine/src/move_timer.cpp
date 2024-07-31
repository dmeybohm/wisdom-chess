#include "move_timer.hpp"

#include <iostream>

namespace wisdom
{
    using chrono::steady_clock;

    static constexpr int Num_Iterations_Before_Checking = 1'000'000;

    auto MoveTimer::isTriggered() -> bool
    {
        if (!my_started_time.has_value())
            return false;

        if (my_triggered || my_cancelled)
            return true;

        if (++my_check_calls % my_current_iterations != 0)
            return false;

        if (my_periodic_function.has_value())
        {
            (*my_periodic_function) (this);
            if (my_triggered || my_cancelled)
                return true;
        }

        steady_clock::time_point check_time = steady_clock::now();
        auto diff_time = check_time - *my_started_time;

        // adjust the the next iteration if less than 250ms:
        if (my_last_check_time.has_value())
        {
            auto last_time = *my_last_check_time;
            auto check_time_diff = check_time - last_time;
            if (
                check_time_diff < chrono::milliseconds(250) &&
                my_current_iterations < Max_Iterations_Before_Checking
            ) {
                my_current_iterations *= 2;
                std::cout << "Doubling iterations: " << my_current_iterations << "\n";
                if (my_current_iterations > Max_Iterations_Before_Checking)
                    my_current_iterations = Max_Iterations_Before_Checking;
                
            } else if (
                check_time_diff > chrono::seconds(1) &&
                my_current_iterations > Min_Iterations_Before_Checking
            ) {
                std::cout << "Halving iterations: " << my_current_iterations << "\n";
                my_current_iterations /= 2;
                if (my_current_iterations < Min_Iterations_Before_Checking)
                    my_current_iterations = Min_Iterations_Before_Checking;
            }
        }

        my_last_check_time = check_time;

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
