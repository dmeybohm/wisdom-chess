#pragma once

#include <cstddef>
#include <type_traits>

#include "wisdom-chess/lib/error.hpp"
#include "wisdom-chess/lib/optional.hpp"
#include "wisdom-chess/lib/types.hpp"

namespace wisdom
{
    // Raw pointers never own; owning pointers are unique_ptr or shared_ptr.
    // A non-owning pointer is named by whether it may be null: nonnull here,
    // nullable below.
    //
    // A nonnull checks for null when constructed, not when dereferenced.
    template <typename T>
    class nonnull
    {
    public:
        // Reports the failure and aborts when null.
        constexpr nonnull (T* ptr) noexcept // lint-allow(raw-pointer): wraps a raw pointer
            : my_ptr { ptr }
        {
            EXPECTS( ptr != nullptr );
        }

        template <typename U>
            requires std::is_convertible_v<U*, T*> // lint-allow(raw-pointer): type trait
        constexpr nonnull (nonnull<U> other) noexcept
            : my_ptr { other.get() }
        {
        }

        nonnull (std::nullptr_t) = delete;

        // An empty optional<nonnull> holds a null pointer, which only the
        // optional can make.
        struct NullRepresentation
        {
            [[nodiscard]] static constexpr auto
            make (NullRepresentationKey) noexcept
                -> nonnull
            {
                return nonnull { NullTag {} };
            }

            [[nodiscard]] static constexpr auto
            isNull (const nonnull& ptr) noexcept
                -> bool
            {
                return ptr.my_ptr == nullptr;
            }
        };

        // For an API that takes a raw pointer.
        [[nodiscard]] constexpr auto
        get() const noexcept
            -> T* // lint-allow(raw-pointer): unwraps for a raw-pointer API
        {
            return my_ptr;
        }

        constexpr auto
        operator->() const noexcept
            -> T* // lint-allow(raw-pointer): operator-> returns a pointer
        {
            return my_ptr;
        }

        constexpr auto
        operator*() const noexcept
            -> T&
        {
            return *my_ptr;
        }

        [[nodiscard]] constexpr auto
        operator== (const nonnull& other) const noexcept
            -> bool = default;

    private:
        struct NullTag
        {
        };

        explicit constexpr nonnull (NullTag) noexcept
            : my_ptr { nullptr }
        {
        }

        T* my_ptr; // lint-allow(raw-pointer): the wrapped pointer
    };

    // An owning raw pointer, for an object whose deletion is arranged outside
    // C++'s ownership types: Qt's deleteLater() takes it over, or it is
    // returned to JavaScript, which destroys it.
    template <typename T>
    using owning = T*; // lint-allow(raw-pointer): defines the pointer types

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

        constexpr nullable (T* ptr) noexcept // lint-allow(raw-pointer): wraps a raw pointer
            : my_ptr { ptr }
        {
        }

        constexpr nullable (nonnull<T> ptr) noexcept
            : my_ptr { ptr.get() }
        {
        }

        template <typename U>
            requires std::is_convertible_v<U*, T*> // lint-allow(raw-pointer): type trait
        constexpr nullable (nullable<U> other) noexcept
            : my_ptr { other.unsafeGet() }
        {
        }

        [[nodiscard]] constexpr explicit
        operator bool() const noexcept
        {
            return my_ptr != nullptr;
        }

        // A null is a precondition failure.
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
            -> T* // lint-allow(raw-pointer): unwraps for a raw-pointer API
        {
            return my_ptr;
        }

        [[nodiscard]] constexpr auto
        operator== (const nullable& other) const noexcept
            -> bool = default;

    private:
        T* my_ptr = nullptr; // lint-allow(raw-pointer): the wrapped pointer
    };
}
