#include "wisdom-chess/engine/error.hpp"
#include "wisdom-chess/engine/ptr.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "null-nonnull",
        "Precondition failed at .*ptr\\.hpp:[0-9]+: ptr != nullptr"
    )
    {
        int* volatile null_ptr = nullptr; // lint-allow(raw-pointer): the null under test
        [[maybe_unused]] nonnull<int> ptr { null_ptr };
    }

    FATAL_CASE(
        "ensures-noexcept-failure",
        "Postcondition failed at .*fatal_contract_test\\.cpp:[0-9]+: condition"
    )
    {
        volatile bool condition = false;
        ENSURES_NOEXCEPT( condition );
    }

    FATAL_CASE(
        "expects-through-noexcept",
        "Uncaught error: Precondition failed at",
        test::Reports_Uncaught_Errors
    )
    {
        volatile bool condition = false;
        auto checked = [&]() noexcept
        {
            EXPECTS( condition );
        };
        checked();
    }

    // ASSERT() only checks when Debugging is on.
    FATAL_CASE(
        "assert-failure",
        "Assertion failed at .*fatal_contract_test\\.cpp:[0-9]+: condition",
        Debugging
    )
    {
        volatile bool condition = false;
        ASSERT( condition );
    }
}
