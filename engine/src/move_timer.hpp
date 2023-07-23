#ifndef WISDOM_CHESS_MOVE_TIMER_HPP
#define WISDOM_CHESS_MOVE_TIMER_HPP

#include "global.hpp"

namespace wisdom
{
    class MoveTimer
    {
    public:
        using PeriodicFunction = std::function<void(not_null<MoveTimer*>)>;

        explicit MoveTimer (chrono::seconds seconds)
                : my_seconds { seconds }
        {}

        explicit MoveTimer (int seconds)
                : MoveTimer (std::chrono::seconds { seconds })
        {}

        auto isTriggered() -> bool;

        // Whether the search as a whole was cancelled.
        [[nodiscard]] auto isCancelled() const -> bool
        {
            return my_cancelled;
        }

        void start() noexcept
        {
            my_started_time = chrono::steady_clock::now();
        }

        [[nodiscard]] auto seconds() const noexcept -> chrono::seconds
        {
            return my_seconds;
        }

        void setPeriodicFunction (const PeriodicFunction& periodic_function) noexcept
        {
            my_periodic_function = periodic_function;
        }

        void setTriggered (bool triggered) noexcept
        {
            my_triggered = triggered;
        }

        void setCancelled (bool cancelled) noexcept
        {
            my_cancelled = cancelled;
        }

    private:
        chrono::seconds my_seconds;
        std::optional<chrono::steady_clock::time_point> my_started_time {};
        std::optional<PeriodicFunction> my_periodic_function {};
        int my_check_calls = 0;
        bool my_triggered = false;
        bool my_cancelled = false;
    };
}

#endif //WISDOM_CHESS_MOVE_TIMER_HPP
