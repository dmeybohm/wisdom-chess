#include "wisdom-chess/engine/move_timer.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

namespace
{
    // The timer looks at the clock once in so many calls, and never less
    // often than this, so twice that many calls must include a look.
    constexpr int Enough_Calls = Max_Iterations_Before_Checking * 2;

    auto
    callsUntilTriggered (MoveTimer& timer)
        -> optional<int>
    {
        for (int calls = 1; calls <= Enough_Calls; calls++)
        {
            if (timer.isTriggered())
                return calls;
        }
        return nullopt;
    }
}

TEST_CASE( "MoveTimer" )
{
    SUBCASE( "The time budget can be read and changed" )
    {
        MoveTimer timer { 5 };
        CHECK( timer.getTimeLimit() == chrono::seconds { 5 } );

        timer.setTimeLimit (chrono::seconds { 7 });
        CHECK( timer.getTimeLimit() == chrono::seconds { 7 } );

        MoveTimer from_duration { chrono::seconds { 3 } };
        CHECK( from_duration.getTimeLimit() == chrono::seconds { 3 } );

        MoveTimer from_milliseconds { chrono::milliseconds { 250 } };
        CHECK( from_milliseconds.getTimeLimit() == chrono::milliseconds { 250 } );
    }

    SUBCASE( "A budget of less than a second is kept to" )
    {
        MoveTimer timer { chrono::milliseconds { 50 } };
        auto start = chrono::steady_clock::now();
        timer.start();

        while (!timer.isTriggered())
        {
            REQUIRE( chrono::steady_clock::now() - start < chrono::seconds { 5 } );
        }

        auto elapsed = chrono::steady_clock::now() - start;
        CHECK( elapsed >= chrono::milliseconds { 50 } );
        CHECK( elapsed < chrono::milliseconds { 500 } );
    }

    SUBCASE( "A timer that was never started does not trigger" )
    {
        MoveTimer timer { 0 };

        CHECK( !callsUntilTriggered (timer).has_value() );
        CHECK( !timer.isCancelled() );
    }

    SUBCASE( "A spent budget triggers at the first look at the clock" )
    {
        MoveTimer timer { 0 };
        timer.start();

        auto calls = callsUntilTriggered (timer);

        REQUIRE( calls.has_value() );
        CHECK( *calls >= Min_Iterations_Before_Checking );
        CHECK( *calls <= Max_Iterations_Before_Checking );
        CHECK( !timer.isCancelled() );
    }

    SUBCASE( "Once triggered, it stays triggered" )
    {
        MoveTimer timer { 0 };
        timer.start();
        REQUIRE( callsUntilTriggered (timer).has_value() );

        CHECK( timer.isTriggered() );
        CHECK( timer.isTriggered() );
    }

    SUBCASE( "A budget that is not spent does not trigger" )
    {
        MoveTimer timer { chrono::hours { 1 } };
        timer.start();

        CHECK( !callsUntilTriggered (timer).has_value() );
    }

    SUBCASE( "Cancelling triggers at once" )
    {
        MoveTimer timer { chrono::hours { 1 } };
        timer.start();
        timer.setCancelled (true);

        CHECK( timer.isCancelled() );
        CHECK( timer.isTriggered() );
    }

    SUBCASE( "Starting again clears a trigger and a cancellation" )
    {
        MoveTimer timer { chrono::hours { 1 } };
        timer.start();
        timer.setCancelled (true);
        REQUIRE( timer.isTriggered() );

        timer.start();

        CHECK( !timer.isCancelled() );
        CHECK( !timer.isTriggered() );
    }
}

TEST_CASE( "MoveTimer periodic function" )
{
    SUBCASE( "It is called with the timer, as often as the clock is looked at" )
    {
        MoveTimer timer { chrono::hours { 1 } };
        int periodic_calls = 0;
        MoveTimer* seen_timer = nullptr;

        timer.setPeriodicFunction (
            [&] (nonnull_observer_ptr<MoveTimer> the_timer)
            {
                periodic_calls++;
                seen_timer = the_timer;
            }
        );
        timer.start();

        CHECK( !callsUntilTriggered (timer).has_value() );
        CHECK( periodic_calls >= Enough_Calls / Max_Iterations_Before_Checking );
        CHECK( periodic_calls <= Enough_Calls / Min_Iterations_Before_Checking );
        CHECK( seen_timer == &timer );
    }

    SUBCASE( "It is not called before the timer is started" )
    {
        MoveTimer timer { 0 };
        int periodic_calls = 0;

        timer.setPeriodicFunction (
            [&] ([[maybe_unused]] nonnull_observer_ptr<MoveTimer> the_timer)
            {
                periodic_calls++;
            }
        );

        CHECK( !callsUntilTriggered (timer).has_value() );
        CHECK( periodic_calls == 0 );
    }

    SUBCASE( "Cancelling from it ends the search in the same call" )
    {
        MoveTimer timer { chrono::hours { 1 } };
        int periodic_calls = 0;

        timer.setPeriodicFunction (
            [&] (nonnull_observer_ptr<MoveTimer> the_timer)
            {
                periodic_calls++;
                the_timer->setCancelled (true);
            }
        );
        timer.start();

        REQUIRE( callsUntilTriggered (timer).has_value() );
        CHECK( periodic_calls == 1 );
        CHECK( timer.isCancelled() );
    }

    SUBCASE( "Spending the budget from it triggers without cancelling" )
    {
        MoveTimer timer { chrono::hours { 1 } };
        int periodic_calls = 0;

        timer.setPeriodicFunction (
            [&] (nonnull_observer_ptr<MoveTimer> the_timer)
            {
                periodic_calls++;
                the_timer->setTimeLimit (chrono::milliseconds { 0 });
            }
        );
        timer.start();

        REQUIRE( callsUntilTriggered (timer).has_value() );
        CHECK( periodic_calls == 1 );
        CHECK( !timer.isCancelled() );
    }
}

TEST_CASE( "TimingAdjustment" )
{
    auto original = TimingAdjustment::create().getIterations();

    SUBCASE( "It starts within the bounds" )
    {
        CHECK( original >= Min_Iterations_Before_Checking );
        CHECK( original <= Max_Iterations_Before_Checking );
    }

    SUBCASE( "A new timer starts from the last saved value" )
    {
        auto adjustment = TimingAdjustment::create();
        adjustment.setIterations (Min_Iterations_Before_Checking + 123);

        CHECK( adjustment.getIterations() == Min_Iterations_Before_Checking + 123 );
        CHECK( TimingAdjustment::create().getIterations()
               == Min_Iterations_Before_Checking + 123 );
    }

    SUBCASE( "A saved value outside the bounds is clamped for the next timer" )
    {
        auto adjustment = TimingAdjustment::create();

        adjustment.setIterations (1);
        CHECK( TimingAdjustment::create().getIterations() == Min_Iterations_Before_Checking );

        adjustment.setIterations (Max_Iterations_Before_Checking + 1);
        CHECK( TimingAdjustment::create().getIterations() == Max_Iterations_Before_Checking );
    }

    TimingAdjustment::create().setIterations (original);
}
