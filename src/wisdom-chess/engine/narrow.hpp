#pragma once

#include <exception>
#include <stdexcept>
#include <type_traits>

#include <gsl/narrow>

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

    // constexpr version of narrow_cast (no exception at runtime):
    template <typename Target, typename Source> constexpr auto
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

        return gsl::narrow_cast<Target> (value);
    }

    // constexpr version of narrow (exception at runtime):
    template <typename Target, typename Source> constexpr auto
    narrow (Source value)
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
                throw std::runtime_error ("narrow_cast: narrowing occurred");
            }
        }

        return gsl::narrow<Target> (value);
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
