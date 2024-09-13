#pragma once

#include "global.hpp"

namespace wisdom
{
    inline constexpr int Min_Iterations_Before_Checking = 10'000;
    inline constexpr int Max_Iterations_Before_Checking = 1'000'000;

    inline constexpr chrono::milliseconds Lower_Bound_Timer_Check =
        chrono::milliseconds { 100 };
    inline constexpr chrono::milliseconds Upper_Bound_Timer_Check =
        chrono::milliseconds { 150 };

    struct MoveTimer;

    struct TimingAdjustment
    {
        int current_iterations = Min_Iterations_Before_Checking;
    };

    struct TimerState
    {
        optional<chrono::steady_clock::time_point> started_time {};
        optional<chrono::steady_clock::time_point> last_check_time {};

        int check_calls = 0;
        bool triggered = false;
        bool cancelled = false;
    };

    class MoveTimer
    {
    public:
        using PeriodicFunction = std::function<void(not_null<MoveTimer*>)>;

        explicit MoveTimer (chrono::seconds seconds)
            : my_seconds { seconds }
        {
        }

        explicit MoveTimer (int seconds)
            : MoveTimer (std::chrono::seconds { seconds })
        {
        }

        auto isTriggered() -> bool;

        // Whether the search as a whole was cancelled.
        [[nodiscard]] auto isCancelled() const -> bool
        {
            return my_timer_state.cancelled;
        }

        void start() noexcept
        {
            my_timer_state = TimerState {};
            my_timer_state.started_time = chrono::steady_clock::now();
        }

        [[nodiscard]] auto
        getSeconds() const noexcept
            -> chrono::seconds
        {
            return my_seconds;
        }

        void setSeconds (chrono::seconds new_seconds)
        {
            my_seconds = new_seconds;
        }

        void setPeriodicFunction (const PeriodicFunction& periodic_function) noexcept
        {
            my_periodic_function = periodic_function;
        }

        void setTriggered (bool triggered) noexcept
        {
            my_timer_state.triggered = triggered;
        }

        void setCancelled (bool cancelled) noexcept
        {
            my_timer_state.cancelled = cancelled;
        }

    private:
        chrono::seconds my_seconds;

        shared_ptr<TimingAdjustment> my_timing_adjustment = make_shared<TimingAdjustment>();
        optional<PeriodicFunction> my_periodic_function {};

        TimerState my_timer_state {};
    };
}
