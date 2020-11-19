//
// Created by David Meybohm on 9/28/20.
//

#include "timer.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;

enum {
    TIMER_CHECK_COUNT = 50000    // number of iterations before checking
};

bool timer::is_triggered()
{
    if (this->triggered)
        return true;

    if (++this->check_calls % TIMER_CHECK_COUNT != 0)
        return false;

    high_resolution_clock::time_point next_check_time = high_resolution_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(next_check_time - this->last_check_time);

    if (time_span >= this->seconds)
    {
        this->triggered = true;
        return true;
    }
    else
    {
        return false;
    }
}
