#pragma once

#include <concepts>
#include <optional>
#include <type_traits>

#include "wisdom-chess/engine/error.hpp"

namespace wisdom
{
    // A type opts in to a compact optional by giving a value its own API
    // never produces, which the optional uses to mean empty, so it needs no
    // flag of its own. A class nests a public NullRepresentation:
    //
    //     struct NullRepresentation
    //     {
    //         static constexpr auto make() noexcept -> T;
    //         static constexpr auto isNull (const T& value) noexcept -> bool;
    //     };
    //
    // A type that cannot nest one, such as an enum, declares a function in
    // its namespace instead, and compares equal to its result when empty:
    //
    //     constexpr auto nullRepresentation (std::type_identity<T>) noexcept -> T;
    //
    // A private NullRepresentation is not found, and the optional silently
    // falls back to std::optional. Pin the representation after the type
    // with:
    //
    //     static_assert (sizeof (optional<T>) == sizeof (T));
    template <typename T>
    concept HasNestedNullRepresentation = requires (const T& value) {
        { T::NullRepresentation::make() } noexcept -> std::same_as<T>;
        { T::NullRepresentation::isNull (value) } noexcept -> std::same_as<bool>;
    };

    template <typename T>
    concept HasNullRepresentationFunction = std::equality_comparable<T>
        && requires {
            { nullRepresentation (std::type_identity<T> {}) } noexcept -> std::same_as<T>;
        };

    template <typename T>
    concept HasNullRepresentation =
        HasNestedNullRepresentation<T> || HasNullRepresentationFunction<T>;

    template <typename T>
    struct NullRepresentationOf;

    template <HasNestedNullRepresentation T>
    struct NullRepresentationOf<T> : T::NullRepresentation
    {
    };

    template <HasNullRepresentationFunction T>
        requires (!HasNestedNullRepresentation<T>)
    struct NullRepresentationOf<T>
    {
        [[nodiscard]] static constexpr auto
        make() noexcept
            -> T
        {
            return nullRepresentation (std::type_identity<T> {});
        }

        [[nodiscard]] static constexpr auto
        isNull (const T& value) noexcept
            -> bool
        {
            return value == make();
        }
    };

    template <typename T>
    class compact_optional
    {
        static_assert (HasNullRepresentation<T>);
        using Null = NullRepresentationOf<T>;

    public:
        using value_type = T;

        constexpr compact_optional() noexcept = default;

        constexpr compact_optional (std::nullopt_t) noexcept
        {
        }

        constexpr compact_optional (const T& value) noexcept
            : my_value { value }
        {
            ASSERT( !Null::isNull (value) );
        }

        constexpr auto
        operator= (std::nullopt_t) noexcept
            -> compact_optional&
        {
            reset();
            return *this;
        }

        [[nodiscard]] constexpr auto
        has_value() const noexcept
            -> bool
        {
            return !Null::isNull (my_value);
        }

        [[nodiscard]] constexpr explicit
        operator bool() const noexcept
        {
            return has_value();
        }

        // Aborts when empty.
        [[nodiscard]] constexpr auto
        value() const noexcept
            -> const T&
        {
            EXPECTS( has_value() );
            return my_value;
        }

        [[nodiscard]] constexpr auto
        value_or (const T& fallback) const noexcept
            -> T
        {
            return has_value() ? my_value : fallback;
        }

        [[nodiscard]] constexpr auto
        operator*() const noexcept
            -> const T&
        {
            ASSERT( has_value() );
            return my_value;
        }

        [[nodiscard]] constexpr auto
        operator->() const noexcept
            -> const T* // lint-allow(raw-pointer): operator-> returns a pointer
        {
            ASSERT( has_value() );
            return &my_value;
        }

        constexpr void
        reset() noexcept
        {
            my_value = Null::make();
        }

        constexpr auto
        emplace (const T& value) noexcept
            -> const T&
        {
            ASSERT( !Null::isNull (value) );
            my_value = value;
            return my_value;
        }

        [[nodiscard]] friend constexpr auto
        operator== (const compact_optional& first, const compact_optional& second) noexcept
            -> bool
        {
            if (first.has_value() != second.has_value())
                return false;
            return !first.has_value() || first.my_value == second.my_value;
        }

        [[nodiscard]] friend constexpr auto
        operator== (const compact_optional& opt, std::nullopt_t) noexcept
            -> bool
        {
            return !opt.has_value();
        }

        [[nodiscard]] friend constexpr auto
        operator== (const compact_optional& opt, const T& value) noexcept
            -> bool
        {
            return opt.has_value() && opt.my_value == value;
        }

    private:
        T my_value = Null::make();
    };

    // std::optional, or compact_optional for a type with a NullRepresentation.
    template <typename T>
    using optional = std::conditional_t<
        HasNullRepresentation<T>,
        compact_optional<T>,
        std::optional<T>
    >;
}
