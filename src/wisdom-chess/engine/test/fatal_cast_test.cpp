#include "wisdom-chess/engine/cast.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "narrow-overflow",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: narrow: the value fits"
    )
    {
        int volatile too_big = 300;
        [[maybe_unused]] auto narrowed = narrow<int8_t> (too_big);
    }

    FATAL_CASE(
        "narrow-negative-to-unsigned",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: narrow: the value fits"
    )
    {
        int volatile negative = -1;
        [[maybe_unused]] auto narrowed = narrow<std::size_t> (negative);
    }

    FATAL_CASE(
        "to-unsigned-negative",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: to_unsigned: the value is nonnegative"
    )
    {
        int volatile negative = -1;
        [[maybe_unused]] auto converted = to_unsigned<std::size_t> (negative);
    }

    FATAL_CASE(
        "to-enum-overflow",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: to_enum: the value fits"
    )
    {
        enum class Small : int8_t
        {
            Value
        };
        int volatile too_big = 300;
        [[maybe_unused]] auto converted = to_enum<Small> (too_big);
    }

    // The _debug conversions only check when Debugging is on.
    FATAL_CASE(
        "narrow-debug-overflow",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: narrow_debug: the value fits",
        Debugging
    )
    {
        int volatile too_big = 300;
        [[maybe_unused]] auto narrowed = narrow_debug<int8_t> (too_big);
    }

    FATAL_CASE(
        "to-unsigned-debug-negative",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: to_unsigned_debug: the value is nonnegative",
        Debugging
    )
    {
        int volatile negative = -1;
        [[maybe_unused]] auto converted = to_unsigned_debug<std::size_t> (negative);
    }

    FATAL_CASE(
        "to-enum-debug-overflow",
        "Precondition failed at .*fatal_cast_test\\.cpp:[0-9]+: to_enum_debug: the value fits",
        Debugging
    )
    {
        enum class Small : int8_t
        {
            Value
        };
        int volatile too_big = 300;
        [[maybe_unused]] auto converted = to_enum_debug<Small> (too_big);
    }
}
