#pragma once

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

    // Aborts, naming the caller, when the value does not fit in Target. In a
    // constant expression, that is a compile error.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    narrow (Source value, std::source_location location = std::source_location::current()) noexcept
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        if (!isLosslessConversion<Target> (value)) [[unlikely]]
            terminateOnCheckFailure ("Precondition", "narrow: the value fits in the target type", location);

        return static_cast<Target> (value);
    }

    // Like narrow(), but checked only when Debugging is on or in a
    // constant expression, as ASSERT() is. Release and RelWithDebInfo builds
    // do a plain static_cast.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    narrow_debug (Source value, std::source_location location = std::source_location::current()) noexcept
        -> Target
    {
        static_assert (std::is_arithmetic_v<Source>);
        static_assert (std::is_arithmetic_v<Target>);

        if (Debugging || std::is_constant_evaluated())
        {
            if (!isLosslessConversion<Target> (value)) [[unlikely]]
                terminateOnCheckFailure ("Precondition", "narrow_debug: the value fits in the target type", location);
        }

        return static_cast<Target> (value);
    }

    // Converts to a wider integer type that holds every value of the source,
    // so it cannot fail. A signed source needs a signed target; to_unsigned()
    // converts a signed value to an unsigned type.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    widen (Source value) noexcept
        -> Target
    {
        static_assert (std::is_integral_v<Source> && std::is_integral_v<Target>);
        static_assert (sizeof (Target) > sizeof (Source));
        static_assert (std::is_unsigned_v<Source> || std::is_signed_v<Target>);

        return static_cast<Target> (value);
    }

    // Converts a nonnegative signed value to an unsigned type at least as
    // wide. Aborts, naming the caller, for a negative value.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    to_unsigned (Source value, std::source_location location = std::source_location::current()) noexcept
        -> Target
    {
        static_assert (std::is_integral_v<Source> && std::is_signed_v<Source>);
        static_assert (std::is_integral_v<Target> && std::is_unsigned_v<Target>);
        static_assert (sizeof (Target) >= sizeof (Source));

        if (value < 0) [[unlikely]]
            terminateOnCheckFailure ("Precondition", "to_unsigned: the value is nonnegative", location);

        return static_cast<Target> (value);
    }

    // Like to_unsigned(), but checked only when Debugging is on or in
    // a constant expression, as ASSERT() is. Release and RelWithDebInfo builds
    // do a plain static_cast.
    template <typename Target, typename Source>
    [[nodiscard]] constexpr auto
    to_unsigned_debug (Source value, std::source_location location = std::source_location::current()) noexcept
        -> Target
    {
        static_assert (std::is_integral_v<Source> && std::is_signed_v<Source>);
        static_assert (std::is_integral_v<Target> && std::is_unsigned_v<Target>);
        static_assert (sizeof (Target) >= sizeof (Source));

        if (Debugging || std::is_constant_evaluated())
        {
            if (value < 0) [[unlikely]]
                terminateOnCheckFailure ("Precondition", "to_unsigned_debug: the value is nonnegative", location);
        }

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

    // Converts an enum to its underlying type, as C++23's std::to_underlying.
    template <typename Enum>
    [[nodiscard]] constexpr auto
    to_underlying (Enum value) noexcept
        -> std::underlying_type_t<Enum>
    {
        static_assert (std::is_enum_v<Enum>);

        return static_cast<std::underlying_type_t<Enum>> (value);
    }
}
