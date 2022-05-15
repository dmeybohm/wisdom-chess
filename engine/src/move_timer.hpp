#ifndef WISDOM_CHESS_TIMER_HPP
#define WISDOM_CHESS_TIMER_HPP

#include "global.hpp"

namespace wisdom
{
    class MoveTimer
    {
    public:
        using PeriodicFunction = std::function<void(gsl::not_null<MoveTimer*>)>;

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

        auto is_triggered () -> bool;

        void start () noexcept
        {
            my_started = true;
        }

        [[nodiscard]] auto seconds () const noexcept -> chrono::seconds
        {
            return my_seconds;
        }

        void set_periodic_function (PeriodicFunction periodic_function) noexcept
        {
            my_periodic_function = std::move (periodic_function);
        }

        void set_triggered (bool triggered) noexcept
        {
            my_triggered = triggered;
        }

    private:
        chrono::steady_clock::time_point my_last_check_time;
        chrono::seconds my_seconds;
        optional<PeriodicFunction> my_periodic_function {};
        int my_check_calls = 0;
        bool my_triggered = false;
        bool my_started = true;
    };
}

#endif //WISDOM_CHESS_TIMER_HPP
