#ifndef WISDOM_TIMER_HPP
#define WISDOM_TIMER_HPP

#include <chrono>

namespace wisdom
{
    class MoveTimer
    {
    private:
        std::chrono::high_resolution_clock::time_point my_last_check_time;
        int my_check_calls = 0;
        std::chrono::seconds my_seconds;
        bool my_triggered = false;
        bool my_started = true;

    public:

        explicit MoveTimer (std::chrono::seconds seconds) :
                my_last_check_time { std::chrono::high_resolution_clock::now () },
                my_seconds { seconds }
        {}

        explicit MoveTimer (int seconds) :
                MoveTimer( std::chrono::seconds { seconds } )
        {}

        MoveTimer (int seconds, bool autostart) :
                my_last_check_time { std::chrono::high_resolution_clock::now () },
                my_seconds { seconds },
                my_started { autostart }
        {}

        bool is_triggered ();

        void start() noexcept
        {
            my_started = true;
        }

        [[nodiscard]] std::chrono::seconds seconds() const noexcept
        {
            return my_seconds;
        }
    };
}
#endif //WISDOM_TIMER_HPP
