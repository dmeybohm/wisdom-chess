#include "wisdom-chess/lib/error.hpp"
#include "wisdom-chess/lib/ptr.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "null-nonnull",
        R"re(Precondition failed at .*ptr\.hpp:[0-9]+: ptr != nullptr)re"
    )
    {
        int* volatile null_ptr = nullptr; // lint-allow(raw-pointer): the null under test
        [[maybe_unused]] nonnull<int> ptr { null_ptr };
    }

    FATAL_CASE(
        "null-nullable-value",
        R"re(Precondition failed at .*ptr\.hpp:[0-9]+: my_ptr != nullptr)re"
    )
    {
        nullable<int> null_ptr;
        [[maybe_unused]] nonnull<int> ptr = null_ptr.value();
    }

    FATAL_CASE(
        "expects-failure",
        R"re(Precondition failed at .*fatal_contract_test\.cpp:[0-9]+: condition)re"
    )
    {
        volatile bool condition = false;
        EXPECTS( condition );
    }

    FATAL_CASE(
        "ensures-failure",
        R"re(Postcondition failed at .*fatal_contract_test\.cpp:[0-9]+: condition)re"
    )
    {
        volatile bool condition = false;
        ENSURES( condition );
    }

    FATAL_CASE(
        "precondition-failed",
        R"re(Precondition failed at .*fatal_contract_test\.cpp:[0-9]+: a test description)re"
    )
    {
        PRECONDITION_FAILED( "a test description" );
    }

    // ASSERT() only checks when Debugging is on.
    FATAL_CASE(
        "assert-failure",
        R"re(Assertion failed at .*fatal_contract_test\.cpp:[0-9]+: condition)re",
        Debugging
    )
    {
        volatile bool condition = false;
        ASSERT( condition );
    }
}
