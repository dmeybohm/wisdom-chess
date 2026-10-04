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
        "null-nullable-value",
        "Precondition failed at .*ptr\\.hpp:[0-9]+: my_ptr != nullptr"
    )
    {
        nullable<int> null_ptr;
        [[maybe_unused]] nonnull<int> ptr = null_ptr.value();
    }

    FATAL_CASE(
        "expects-failure",
        "Precondition failed at .*fatal_contract_test\\.cpp:[0-9]+: condition"
    )
    {
        volatile bool condition = false;
        EXPECTS( condition );
    }

    FATAL_CASE(
        "ensures-failure",
        "Postcondition failed at .*fatal_contract_test\\.cpp:[0-9]+: condition"
    )
    {
        volatile bool condition = false;
        ENSURES( condition );
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
