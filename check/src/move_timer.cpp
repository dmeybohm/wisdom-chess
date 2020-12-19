//
// Created by David Meybohm on 9/28/20.
//

#include "move_timer.h"

using std::chrono::high_resolution_clock;

enum {
    Num_Calls_Per_Timer_Check = 10000    // number of iterations before checking
};

bool move_timer::is_triggered ()
{
    if (this->triggered)
        return true;

    if (++this->check_calls % Num_Calls_Per_Timer_Check != 0)
        return false;

    high_resolution_clock::time_point next_check_time = high_resolution_clock::now();
    auto diff_time = next_check_time - this->last_check_time;

    if (diff_time >= this->seconds)
    {
        this->triggered = true;
        return true;
    }
    else
    {
        return false;
    }
}
