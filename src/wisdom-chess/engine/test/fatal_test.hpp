#pragma once

// Each case triggers one fatal error, so it has to run in its own process.
// run_fatal_test.cmake launches it and checks the output and the exit result.
// To add a case, write it with FATAL_CASE() in a source file of a program
// made with wisdom_chess_add_fatal_tests() (cmake/FatalTests.cmake): the build
// asks the program for the list (discover_fatal_tests.cmake).

#include <iostream>
#include <span>
#include <string_view>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/ptr.hpp"

namespace wisdom::test
{
    // Prints only what the emergency logger reports, marked so that
    // run_fatal_test.cmake can find it.
    struct MarkedLogger : Logger
    {
        void debug ([[maybe_unused]] const string& output) noexcept override
        {
        }

        void info ([[maybe_unused]] const string& output) noexcept override
        {
        }

        void emergency (string_view output) noexcept override
        {
            std::cout << "[emergency] " << output << std::endl;
        }
    };

    // Under Emscripten an uncaught exception leaves main() for JavaScript
    // without calling std::terminate(), and one thrown through noexcept
    // reaches the terminate handler with no current exception to report.
#ifdef __EMSCRIPTEN__
    constexpr bool Reports_Uncaught_Errors = false;
#else
    constexpr bool Reports_Uncaught_Errors = true;
#endif

    struct FatalCase
    {
        nonnull<void()> run;

        string_view name;

        // A CMake regular expression for what the emergency logger reports.
        string_view expected;

        // Whether this build reports the error at all.
        bool listed = true;
    };

    // The cases registered so far, in no particular order.
    [[nodiscard]] auto fatalCases() noexcept -> std::span<const FatalCase>;

    struct FatalCaseRegistrar
    {
        explicit FatalCaseRegistrar (const FatalCase& fatal_case) noexcept;
    };
}

#define WISDOM_FATAL_CASE_CONCAT_INNER(a, b) a##b
#define WISDOM_FATAL_CASE_CONCAT(a, b) WISDOM_FATAL_CASE_CONCAT_INNER (a, b)

#define WISDOM_FATAL_CASE_IMPL(function, ...) \
    static void function(); \
    static const ::wisdom::test::FatalCaseRegistrar \
        WISDOM_FATAL_CASE_CONCAT (function, _registrar) { \
            ::wisdom::test::FatalCase { &function, __VA_ARGS__ } \
        }; \
    static void function()

// Defines a case, written like doctest's TEST_CASE() and followed by its
// body: FATAL_CASE( name, expected [, listed] ). The expected message and
// the listed flag are those of FatalCase.
#define FATAL_CASE(...) \
    WISDOM_FATAL_CASE_IMPL (WISDOM_FATAL_CASE_CONCAT (fatalCase, __LINE__), __VA_ARGS__)
