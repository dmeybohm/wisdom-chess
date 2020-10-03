//
// Created by David Meybohm on 9/28/20.
//

#include "global.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>

#define TIMER_CHECK_COUNT      50000    /* number of iterations before checking */

void timer_init (struct timer *timer, int seconds)
{
    if (gettimeofday (&timer->last_check_time, NULL) == -1)
    {
        perror ("gettimeofday");
        exit (1);
    }
    timer->check_calls = 0;
    timer->seconds = (double)seconds;
    timer->triggered = false;
}

bool timer_is_triggered (struct timer *timer)
{
    struct timeval next_time;

    if (timer->triggered)
        return true;

    if (++timer->check_calls % TIMER_CHECK_COUNT != 0)
        return false;

    if (gettimeofday (&next_time, NULL) == -1)
    {
        perror ("gettimeofday");
        exit (1);
    }

    double seconds = difftime (next_time.tv_sec, timer->last_check_time.tv_sec);

    if (seconds >= timer->seconds)
    {
        timer->triggered = true;
        return true;
    }
    else
    {
        return false;
    }
}
