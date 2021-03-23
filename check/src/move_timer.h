#ifndef CHECK_TIMER_H
#define CHECK_TIMER_H

#include <chrono>

struct MoveTimer
{
    std::chrono::high_resolution_clock::time_point last_check_time;
    int check_calls;
    std::chrono::seconds seconds;
    bool triggered;

    explicit MoveTimer (int _seconds) :
        last_check_time { std::chrono::high_resolution_clock::now() },
        check_calls { 0 },
        seconds { _seconds },
        triggered { false }
    {
    }

    explicit MoveTimer (std::chrono::seconds _seconds) :
        last_check_time { std::chrono::high_resolution_clock::now() },
        check_calls { 0 },
        seconds { _seconds },
        triggered { false }
    {
    }

    bool is_triggered();
};

#endif //CHECK_TIMER_H
