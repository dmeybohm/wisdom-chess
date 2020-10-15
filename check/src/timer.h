#ifndef CHECK_TIMER_H
#define CHECK_TIMER_H

#include "config.h"

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

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
