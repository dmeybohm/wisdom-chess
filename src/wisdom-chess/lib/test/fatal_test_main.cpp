#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

#include "fatal_test.hpp"

using namespace wisdom;
using wisdom::test::FatalCase;

namespace
{
    // A function-local static, so a case in another file can register
    // before main() whatever the order of static initialization.
    auto registeredCases() noexcept -> std::vector<FatalCase>&
    {
        static std::vector<FatalCase> cases;
        return cases;
    }

#ifndef WISDOM_CHESS_FILC_COMPAT
    // Exits with a failure status, which saves writing a core dump per case.
    // The test does not depend on this: FIL-C turns abort() into a trap that
    // cannot be caught, and the runner script accepts either ending.
    extern "C" void onAbort ([[maybe_unused]] int signal_number)
    {
        std::_Exit (EXIT_FAILURE);
    }
#endif
}

namespace wisdom::test
{
    auto fatalCases() noexcept -> std::span<const FatalCase>
    {
        return registeredCases();
    }

    FatalCaseRegistrar::FatalCaseRegistrar (const FatalCase& fatal_case) noexcept
    {
        registeredCases().push_back (fatal_case);
    }
}

auto
main (int argc, char* argv[]) // lint-allow(raw-pointer): main's signature
    -> int
{
#ifdef _MSC_VER
    // No dialog box or error report when the case aborts.
    _set_abort_behavior (0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifndef WISDOM_CHESS_FILC_COMPAT
    std::signal (SIGABRT, onAbort);
#endif

    setEmergencyLogger (std::make_shared<test::MarkedLogger>());
    installEmergencyTerminateHandler();

    std::string_view requested = argc > 1 ? argv[1] : "";

    if (requested == "--list")
    {
        std::vector<FatalCase> cases { test::fatalCases().begin(), test::fatalCases().end() };
        std::ranges::sort (cases, {}, &FatalCase::name);

        auto duplicate = std::ranges::adjacent_find (cases, {}, &FatalCase::name);
        if (duplicate != cases.end())
        {
            std::cout << "Duplicate case: " << duplicate->name << "\n";
            return EXIT_FAILURE;
        }

        for (const auto& fatal_case : cases)
        {
            if (fatal_case.listed)
                std::cout << fatal_case.name << "\n";
        }
        return EXIT_SUCCESS;
    }

    for (const auto& fatal_case : test::fatalCases())
    {
        if (fatal_case.name != requested)
            continue;

        std::cout << "[expecting] " << fatal_case.expected << std::endl;
        (*fatal_case.run)();
        std::cout << "[survived] " << requested << "\n";
        return EXIT_SUCCESS;
    }

    std::cout << "Unknown case: " << requested << "\n";
    return EXIT_FAILURE;
}
