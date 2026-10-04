#pragma once

#include <exception>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "wisdom-chess/engine/types.hpp"

namespace wisdom
{
    // Whether ASSERT() checks its condition. Follows NDEBUG, as assert()
    // does, so Release and RelWithDebInfo builds compile the checks out.
#ifdef NDEBUG
    inline constexpr bool Debugging = false;
#else
    inline constexpr bool Debugging = true;
#endif

    // Errors in this application.
    class Error : public std::exception
    {
    private:
        struct Text
        {
            string message;
            string extra_info;
        };

        // Shared, so that copying the exception cannot throw.
        shared_ptr<const Text> my_text;

    public:
        Error (string message, string extra_info)
            : my_text {
                make_shared<const Text> (Text { std::move (message), std::move (extra_info) })
            }
        {
        }

        explicit Error (string message)
            : Error (std::move (message), "")
        {
        }

        // Declared so that there is no move, which would leave my_text empty.
        Error (const Error& src) noexcept = default;
        auto operator= (const Error& src) noexcept -> Error& = default;

        [[nodiscard]] auto
        message() const noexcept
            -> const string&
        {
            return my_text->message;
        }

        [[nodiscard]] auto
        extraInfo() const noexcept
            -> const string&
        {
            return my_text->extra_info;
        }

        [[nodiscard]] auto
        what() const noexcept
            -> czstring override
        {
            return my_text->message.c_str();
        }
    };

    // Reports through logEmergency() and aborts. The kind names the check
    // that failed, such as "Precondition". Allocates nothing: the message
    // is built on the stack, since the heap may be what failed.
    [[noreturn]] void
    terminateOnCheckFailure (
        string_view kind,
        string_view expression,
        std::source_location location
    ) noexcept;

    // The checks below are called through the macros at the end of this
    // file, which supply the text of the condition. In a constant
    // expression, a false condition is a compile error instead of an abort.

    // Reports the failure and aborts when the condition is false.
    constexpr void
    expects (
        bool condition,
        string_view expression,
        std::source_location location = std::source_location::current()
    ) noexcept
    {
        if (!condition) [[unlikely]]
            terminateOnCheckFailure ("Precondition", expression, location);
    }

    // Reports the failure and aborts when the condition is false.
    constexpr void
    ensures (
        bool condition,
        string_view expression,
        std::source_location location = std::source_location::current()
    ) noexcept
    {
        if (!condition) [[unlikely]]
            terminateOnCheckFailure ("Postcondition", expression, location);
    }

    // Reports the failure and aborts when the condition is false. ASSERT()
    // calls it only when Debugging is on or in a constant expression.
    constexpr void
    debug_expects (
        bool condition,
        string_view expression,
        std::source_location location = std::source_location::current()
    ) noexcept
    {
        if (!condition) [[unlikely]]
            terminateOnCheckFailure ("Assertion", expression, location);
    }
}

// Written with spaces inside the parentheses, like the test macros:
// EXPECTS( index < size ). The failure message quotes the condition.
#define EXPECTS(condition) ::wisdom::expects ((condition), #condition)
#define ENSURES(condition) ::wisdom::ensures ((condition), #condition)

// A replacement for assert(): in a build without Debugging the condition is
// type-checked but not evaluated, except in a constant expression, where a
// false condition is a compile error in every build.
#define ASSERT(condition) \
    (::wisdom::Debugging || std::is_constant_evaluated() \
         ? ::wisdom::debug_expects ((condition), #condition) \
         : void())
