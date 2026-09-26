#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // How many calls to isTriggered() pass between looks at the clock. Small
    // enough that a search keeps to a budget of a few milliseconds, and
    // large enough that reading the clock costs nothing measurable, even
    // where that is a system call. A power of two, so the test is a mask.
    inline constexpr int Calls_Between_Clock_Checks = 1024;
    static_assert ((Calls_Between_Clock_Checks & (Calls_Between_Clock_Checks - 1)) == 0);

    struct TimerState
    {
        optional<chrono::steady_clock::time_point> started_time {};

        int check_calls = 0;
        bool triggered = false;
        bool cancelled = false;
    };

    class MoveTimer
    {
    public:
        using PeriodicFunction = std::function<void(nonnull_observer_ptr<MoveTimer>)>;

        explicit MoveTimer (chrono::milliseconds time_limit)
            : my_time_limit { time_limit }
        {
        }

        explicit MoveTimer (int seconds)
            : MoveTimer (std::chrono::seconds { seconds })
        {
        }

        auto isTriggered() -> bool;

        // Whether the search as a whole was cancelled.
        [[nodiscard]] auto 
        isCancelled() const 
            -> bool
        {
            return my_timer_state.cancelled;
        }

        void start() noexcept
        {
            my_timer_state = TimerState {};
            my_timer_state.started_time = chrono::steady_clock::now();
        }

        [[nodiscard]] auto
        getTimeLimit() const noexcept
            -> chrono::milliseconds
        {
            return my_time_limit;
        }

        void setTimeLimit (chrono::milliseconds time_limit)
        {
            my_time_limit = time_limit;
        }

        void setPeriodicFunction (const PeriodicFunction& periodic_function) noexcept
        {
            my_periodic_function = periodic_function;
        }

        void setCancelled (bool cancelled) noexcept
        {
            my_timer_state.cancelled = cancelled;
            if (cancelled)
                my_timer_state.triggered = true;
        }

    private:
        void setTriggered (bool triggered) noexcept
        {
            my_timer_state.triggered = triggered;
        }

        chrono::milliseconds my_time_limit;

        optional<PeriodicFunction> my_periodic_function {};

        TimerState my_timer_state {};
    };
}
