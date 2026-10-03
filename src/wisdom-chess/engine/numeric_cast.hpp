#pragma once

#include <exception>
#include <source_location>
#include <type_traits>

#include "wisdom-chess/engine/error.hpp"

namespace wisdom
{
    template <typename T>
    [[nodiscard]] constexpr auto
    isNegative (T value) noexcept
        -> bool
    {
        if constexpr (std::is_signed_v<T>)
            return value < T {};
        else
            return false;
    }

    // Whether converting the value to Target and back preserves it.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    isLosslessConversion (Source value) noexcept
        -> bool
    {
        auto converted = static_cast<Target> (value);
        return static_cast<Source> (converted) == value
            && isNegative (converted) == isNegative (value);
    }

    // A static_cast that names a narrowing conversion. Unchecked at runtime;
    // in a constant expression, a value that does not fit is a compile error.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    narrow_cast (Source value) noexcept
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        // Check if Source can fit into Target without truncation
        if (std::is_constant_evaluated())
        {
            if (!isLosslessConversion<Target> (value))
            {
                // At compile-time, trigger an error if there's truncation
                std::terminate();
            }
        }

        return static_cast<Target> (value);
    }

    // Throws PreconditionError, naming the caller, when the value does not
    // fit in Target. In a constant expression, that is a compile error.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    narrow (Source value, std::source_location location = std::source_location::current())
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        if (!isLosslessConversion<Target> (value)) [[unlikely]]
            throwPreconditionError ("narrow: the value fits in the target type", location);

        return static_cast<Target> (value);
    }

    // Like narrow(), but aborts instead of throwing, naming the caller. For an
    // invariant inside a noexcept function.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    noexcept_narrow (Source value, std::source_location location = std::source_location::current()) noexcept
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        if (!isLosslessConversion<Target> (value)) [[unlikely]]
            terminateOnCheckFailure ("Precondition", "noexcept_narrow: the value fits in the target type", location);

        return static_cast<Target> (value);
    }

    // Converts to a strictly wider integer type without a runtime check.
    // A signed negative value converted to an unsigned type wraps; in a
    // constant expression, a value that does not fit is a compile error.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    widen_cast (Source value) noexcept
        -> Target
    {
        static_assert (std::is_integral_v<Source> && std::is_integral_v<Target>);
        static_assert (sizeof (Target) > sizeof (Source));

        if (std::is_constant_evaluated())
        {
            if (!isLosslessConversion<Target> (value))
                std::terminate();
        }

        return static_cast<Target> (value);
    }

    // Converts to a strictly wider integer type, rejecting values that do
    // not fit (including negative values when Target is unsigned).
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    widen (Source value, std::source_location location = std::source_location::current())
        -> Target
    {
        static_assert (std::is_integral_v<Source> && std::is_integral_v<Target>);
        static_assert (sizeof (Target) > sizeof (Source));

        if (!isLosslessConversion<Target> (value)) [[unlikely]]
            throwPreconditionError ("widen: the value fits in the target type", location);

        return static_cast<Target> (value);
    }

    // Like widen(), but aborts instead of throwing for a failed invariant.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    noexcept_widen (Source value, std::source_location location = std::source_location::current()) noexcept
        -> Target
    {
        static_assert (std::is_integral_v<Source> && std::is_integral_v<Target>);
        static_assert (sizeof (Target) > sizeof (Source));

        if (!isLosslessConversion<Target> (value)) [[unlikely]]
            terminateOnCheckFailure ("Precondition", "noexcept_widen: the value fits in the target type", location);

        return static_cast<Target> (value);
    }

    // Converts to a narrower unsigned type, deliberately discarding the high bits.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    truncate (Source value) noexcept
        -> Target
    {
        static_assert (std::is_unsigned_v<Source> && std::is_unsigned_v<Target>);
        static_assert (sizeof (Target) <= sizeof (Source));

        return static_cast<Target> (value);
    }
}
