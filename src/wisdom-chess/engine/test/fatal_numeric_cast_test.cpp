#include "wisdom-chess/engine/numeric_cast.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "narrow-noexcept-overflow",
        "Precondition failed at .*fatal_numeric_cast_test\\.cpp:[0-9]+: narrow_noexcept: the value fits"
    )
    {
        int volatile too_big = 300;
        [[maybe_unused]] auto narrowed = narrow_noexcept<int8_t> (too_big);
    }

    FATAL_CASE(
        "to-unsigned-noexcept-negative",
        "Precondition failed at .*fatal_numeric_cast_test\\.cpp:[0-9]+: to_unsigned_noexcept: the value is nonnegative"
    )
    {
        int volatile negative = -1;
        [[maybe_unused]] auto converted = to_unsigned_noexcept<std::size_t> (negative);
    }

    // The _debug conversions only check when Debugging is on.
    FATAL_CASE(
        "narrow-debug-overflow",
        "Precondition failed at .*fatal_numeric_cast_test\\.cpp:[0-9]+: narrow_debug: the value fits",
        Debugging
    )
    {
        int volatile too_big = 300;
        [[maybe_unused]] auto narrowed = narrow_debug<int8_t> (too_big);
    }

    FATAL_CASE(
        "to-unsigned-debug-negative",
        "Precondition failed at .*fatal_numeric_cast_test\\.cpp:[0-9]+: to_unsigned_debug: the value is nonnegative",
        Debugging
    )
    {
        int volatile negative = -1;
        [[maybe_unused]] auto converted = to_unsigned_debug<std::size_t> (negative);
    }
}
