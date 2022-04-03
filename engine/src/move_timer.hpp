#ifndef WISDOM_TIMER_HPP
#define WISDOM_TIMER_HPP

#include "global.hpp"

namespace wisdom
{
    class MoveTimer;

    struct PeriodicNotified
    {
       virtual void notify (MoveTimer* timer) = 0;
    };

    class MoveTimer
    {
    private:
        chrono::steady_clock::time_point my_last_check_time;
        int my_check_calls = 0;
        chrono::seconds my_seconds;
        bool my_triggered = false;
        bool my_started = true;
        PeriodicNotified* my_periodic_notified = nullptr;

    public:

        explicit MoveTimer (chrono::seconds seconds) :
                my_last_check_time { chrono::steady_clock::now () },
                my_seconds { seconds }
        {}

        explicit MoveTimer (int seconds) :
                MoveTimer( std::chrono::seconds { seconds } )
        {}

        MoveTimer (int seconds, bool autostart) :
                my_last_check_time { chrono::steady_clock::now () },
                my_seconds { seconds },
                my_started { autostart }
        {}

        auto is_triggered () noexcept -> bool;

        void start () noexcept
        {
            my_started = true;
        }

        [[nodiscard]] auto seconds () const noexcept -> chrono::seconds
        {
            return my_seconds;
        }

        void set_periodic_notified (PeriodicNotified* notified)
        {
            my_periodic_notified = notified;
        }

        void set_triggered (bool triggered)
        {
            my_triggered = triggered;
        }
    };
}
#endif //WISDOM_TIMER_HPP
