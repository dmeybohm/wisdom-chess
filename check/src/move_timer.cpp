//
// Created by David Meybohm on 9/28/20.
//

#include "move_timer.hpp"

namespace wisdom
{
    using std::chrono::high_resolution_clock;

    enum
    {
        Num_Calls_Per_Timer_Check = 1000    // number of iterations before checking
    };

    bool MoveTimer::is_triggered ()
    {
        if (my_triggered)
            return true;

        if (++my_check_calls % Num_Calls_Per_Timer_Check != 0)
            return false;

        high_resolution_clock::time_point next_check_time = high_resolution_clock::now ();
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