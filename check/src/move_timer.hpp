#ifndef WISDOM_TIMER_HPP
#define WISDOM_TIMER_HPP

#include <chrono>

namespace wisdom
{
    class MoveTimer
    {
    private:
        std::chrono::high_resolution_clock::time_point my_last_check_time;
        int my_check_calls;
        std::chrono::seconds my_seconds;
        bool my_triggered;

    public:
        explicit MoveTimer (int seconds) :
                my_last_check_time { std::chrono::high_resolution_clock::now () },
                my_check_calls { 0 },
                my_seconds { seconds },
                my_triggered { false }
        {
        }

        explicit MoveTimer (std::chrono::seconds seconds) :
                my_last_check_time { std::chrono::high_resolution_clock::now () },
                my_check_calls { 0 },
                my_seconds { seconds },
                my_triggered { false }
        {
        }

        bool is_triggered ();

        std::chrono::seconds seconds() const noexcept
        {
            return my_seconds;
        }
    };
}
#endif //WISDOM_TIMER_HPP
