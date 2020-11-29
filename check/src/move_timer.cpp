//
// Created by David Meybohm on 9/28/20.
//

#include "move_timer.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

enum {
    TIMER_CHECK_COUNT = 10000    // number of iterations before checking
};

bool move_timer::is_triggered ()
{
    if (this->triggered)
        return true;

    if (++this->check_calls % TIMER_CHECK_COUNT != 0)
        return false;

    high_resolution_clock::time_point next_check_time = high_resolution_clock::now();
    auto diff_time = next_check_time - this->last_check_time;
    duration<double> time_span = duration_cast<duration<double>>(next_check_time - this->last_check_time);

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
