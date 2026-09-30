#pragma once

#include <cstddef>
#include <type_traits>

#include <gsl/gsl>

#include "wisdom-chess/engine/error.hpp"

namespace wisdom
{
    // Raw pointers never own; owning pointers are unique_ptr or shared_ptr.
    // A non-owning pointer is named by whether it may be null: nonnull here,
    // nullable and unchecked_nonnull below.
    template <typename T>
    using nonnull = gsl::not_null<T*>; // lint-allow(raw-pointer): defines the pointer types

    // An owning raw pointer, for an object whose deletion is arranged outside
    // C++'s ownership types: Qt's deleteLater() takes it over, or it is
    // returned to JavaScript, which destroys it.
    template <typename T>
    using owning = gsl::owner<T*>; // lint-allow(raw-pointer): defines the pointer types

    // A non-owning pointer that may be null. It cannot be dereferenced: test
    // it, then take value() to get a nonnull.
    template <typename T>
    class nullable
    {
    public:
        constexpr nullable() noexcept = default;

        constexpr nullable (std::nullptr_t) noexcept
        {
        }

        constexpr nullable (T* ptr) noexcept // lint-allow(raw-pointer)
            : my_ptr { ptr }
        {
        }

        constexpr nullable (nonnull<T> ptr) noexcept
            : my_ptr { ptr.get() }
        {
        }

        template <typename U>
            requires std::is_convertible_v<U*, T*> // lint-allow(raw-pointer)
        constexpr nullable (nullable<U> other) noexcept
            : my_ptr { other.unsafeGet() }
        {
        }

        [[nodiscard]] constexpr explicit
        operator bool() const noexcept
        {
            return my_ptr != nullptr;
        }

        // Throws PreconditionError when null.
        [[nodiscard]] constexpr auto
        value() const
            -> nonnull<T>
        {
            EXPECTS( my_ptr != nullptr );
            return my_ptr;
        }

        // For an API that takes a raw pointer. The result may be null.
        [[nodiscard]] constexpr auto
        unsafeGet() const noexcept
            -> T* // lint-allow(raw-pointer)
        {
            return my_ptr;
        }

        [[nodiscard]] constexpr auto
        operator== (const nullable& other) const noexcept
            -> bool = default;

    private:
        T* my_ptr = nullptr; // lint-allow(raw-pointer)
    };

    // Like nonnull, but checks for null only when constructed, not on each
    // dereference. For pointers dereferenced in a hot loop, where a
    // benchmark shows the check matters.
    template <typename T>
    class unchecked_nonnull
    {
    public:
        constexpr unchecked_nonnull (T* ptr) // lint-allow(raw-pointer)
            : my_ptr { ptr }
        {
            EXPECTS( ptr != nullptr );
        }

        constexpr unchecked_nonnull (nonnull<T> ptr) noexcept
            : my_ptr { ptr.get() }
        {
        }

        constexpr unchecked_nonnull (nullable<T> ptr)
            : my_ptr { ptr.value().get() }
        {
        }

        unchecked_nonnull (std::nullptr_t) = delete;

        [[nodiscard]] constexpr auto
        get() const noexcept
            -> T* // lint-allow(raw-pointer)
        {
            return my_ptr;
        }

        constexpr auto
        operator->() const noexcept
            -> T* // lint-allow(raw-pointer)
        {
            return my_ptr;
        }

        constexpr auto
        operator*() const noexcept
            -> T&
        {
            return *my_ptr;
        }

    private:
        T* my_ptr; // lint-allow(raw-pointer)
    };
}
