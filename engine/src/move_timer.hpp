#pragma once

#include "global.hpp"

namespace wisdom
{
    inline constexpr int Min_Iterations_Before_Checking = 10'000;
    inline constexpr int Max_Iterations_Before_Checking = 1'000'000;

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

        void setCurrentIterations (int current_iterations)
        {
            my_current_iterations = std::max(
                Min_Iterations_Before_Checking,
                std::min(current_iterations, Max_Iterations_Before_Checking)
            );
        }

        [[nodiscard]] auto getCurrentIterations() const -> int
        {
            return my_current_iterations;
        }

    private:
        chrono::seconds my_seconds;
        std::optional<chrono::steady_clock::time_point> my_started_time {};
        std::optional<chrono::steady_clock::time_point> my_last_check_time {};
        std::optional<PeriodicFunction> my_periodic_function {};
        int my_check_calls = 0;
        int my_current_iterations = Min_Iterations_Before_Checking;
        bool my_triggered = false;
        bool my_cancelled = false;
    };
}
