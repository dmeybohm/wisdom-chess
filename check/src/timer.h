//
// Created by David Meybohm on 9/28/20.
//

#ifndef CHECK_TIMER_H
#define CHECK_TIMER_H

#include <sys/time.h>
#include <stdbool.h>

struct timer
{
    struct timeval last_check_time;
    int check_calls;
    double seconds;
    bool triggered;
};

void timer_init (struct timer *timer, int seconds);

bool timer_is_triggered (struct timer *timer);

#endif //CHECK_TIMER_H
