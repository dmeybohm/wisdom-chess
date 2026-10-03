// Each case triggers one fatal error, so it has to run in its own process.
// run_fatal_test.cmake launches it and checks the output and the exit result.
// To add a case, write its function and list it in Fatal_Cases: the build
// asks this program for the list (discover_fatal_tests.cmake).

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/castling.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

using namespace wisdom;

namespace
{
    struct MarkedLogger : Logger
    {
        void debug ([[maybe_unused]] const string& output) const noexcept override
        {
        }

        void info ([[maybe_unused]] const string& output) const noexcept override
        {
        }

        void emergency (string_view output) const noexcept override
        {
            std::cout << "[emergency] " << output << std::endl;
        }
    };

#ifndef WISDOM_CHESS_FILC_COMPAT
    // Exits with a failure status, which saves writing a core dump per case.
    // The test does not depend on this: FIL-C turns abort() into a trap that
    // cannot be caught, and the runner script accepts either ending.
    extern "C" void onAbort ([[maybe_unused]] int signal_number)
    {
        std::_Exit (EXIT_FAILURE);
    }
#endif

    void appendOverflow()
    {
        MoveList list;
        Move move = moveParse ("e2 e4", Color::White);

        for (std::ptrdiff_t i = 0; i <= Max_Move_List_Size; i++)
            list.append (move);
    }

    void removeFromEmpty()
    {
        MoveList list;
        list.removeLast();
    }

    void frontOfEmpty()
    {
        MoveList list;
        [[maybe_unused]] Move move = list.front();
    }

    void backOfEmpty()
    {
        MoveList list;
        [[maybe_unused]] Move move = list.back();
    }

    void badCastlingFlags()
    {
        volatile uint8_t flags = 0x4;
        [[maybe_unused]] CastlingEligibility eligibility { flags };
    }

    void badEnPassantRow()
    {
        Board board { BoardBuilder::fromDefaultPosition() };
        BoardCode code { board };
        code.setEnPassantTarget (
            Color::White,
            makeCoord (Black_En_Passant_Row, 0),
            EnPassantTargetState::Illegal
        );
    }

    void nullNonnull()
    {
        int* volatile null_ptr = nullptr; // lint-allow(raw-pointer): the null under test
        [[maybe_unused]] nonnull<int> ptr { null_ptr };
    }

    void noexceptNarrowOverflow()
    {
        int volatile too_big = 300;
        [[maybe_unused]] auto narrowed = noexcept_narrow<int8_t> (too_big);
    }

    void noexceptWidenNegative()
    {
        int32_t volatile negative = -1;
        [[maybe_unused]] auto widened = noexcept_widen<uint64_t> (negative);
    }

    void needPawnPromotionBadColor()
    {
        Color volatile color = Color::None;
        [[maybe_unused]] bool promote = needPawnPromotion (0, color);
    }

    void uncaughtError()
    {
        throw Error { "boom", "extra detail" };
    }

    // Fails on the timer's first periodic call, which depth 4 reaches.
    void searchError()
    {
        Board board { BoardBuilder::fromDefaultPosition() };
        History history;
        MoveTimer timer { 30 };
        timer.setPeriodicFunction ([] (nonnull<MoveTimer>)
        {
            throw Error { "boom", "extra detail" };
        });
        auto transposition_table = TranspositionTable::fromMegabytes (1);
        auto search = IterativeSearch::create (
            board,
            history,
            std::make_shared<MarkedLogger>(),
            timer,
            4,
            &transposition_table,
            Claimable_Draw_Limits
        );
        (void)search.iterativelyDeepen (Color::White);
    }

    void expectsThroughNoexcept()
    {
        volatile bool condition = false;
        auto checked = [&]() noexcept
        {
            EXPECTS( condition );
        };
        checked();
    }

    void assertFailure()
    {
        volatile bool condition = false;
        ASSERT( condition );
    }

    struct FatalCase
    {
        string_view name;

        // A CMake regular expression for what the emergency logger reports.
        string_view expected;

        nonnull<void()> run;

        // Whether this build reports the error at all.
        bool listed = true;
    };

    // Under Emscripten an uncaught exception leaves main() for JavaScript
    // without calling std::terminate(), and one thrown through noexcept
    // reaches the terminate handler with no current exception to report.
#ifdef __EMSCRIPTEN__
    constexpr bool Reports_Uncaught_Errors = false;
#else
    constexpr bool Reports_Uncaught_Errors = true;
#endif

    const FatalCase Fatal_Cases[] = {
        {
            "append-overflow",
            "Precondition failed at .*move_list\\.hpp:[0-9]+: my_size < Max_Move_List_Size",
            &appendOverflow,
        },
        { "remove-from-empty", "Precondition failed at .*move_list\\.hpp", &removeFromEmpty },
        { "front-of-empty", "Precondition failed at .*move_list\\.hpp", &frontOfEmpty },
        { "back-of-empty", "Precondition failed at .*move_list\\.hpp", &backOfEmpty },
        { "bad-castling-flags", "Precondition failed at .*castling\\.hpp", &badCastlingFlags },
        { "bad-en-passant-row", "Precondition failed at .*board_code\\.hpp", &badEnPassantRow },
        {
            "null-nonnull",
            "Precondition failed at .*ptr\\.hpp:[0-9]+: ptr != nullptr",
            &nullNonnull,
        },
        {
            "noexcept-narrow-overflow",
            "Precondition failed at .*fatal_test_main\\.cpp:[0-9]+: noexcept_narrow: the value fits",
            &noexceptNarrowOverflow,
        },
        {
            "noexcept-widen-negative",
            "Precondition failed at .*fatal_test_main\\.cpp:[0-9]+: noexcept_widen: the value fits",
            &noexceptWidenNegative,
        },
        {
            "need-pawn-promotion-bad-color",
            "Precondition failed at .*generate\\.cpp:[0-9]+: isColorValid \\(who\\)",
            &needPawnPromotionBadColor,
        },
        { "uncaught-error", "Uncaught error: boom", &uncaughtError, Reports_Uncaught_Errors },
        {
            "search-error",
            "Uncaught error: boom.extra detail.[^[]*\\|",
            &searchError,
            Reports_Uncaught_Errors,
        },
        {
            "expects-through-noexcept",
            "Uncaught error: Precondition failed at",
            &expectsThroughNoexcept,
            Reports_Uncaught_Errors,
        },

        // ASSERT() only checks when Debugging is on.
        {
            "assert-failure",
            "Assertion failed at .*fatal_test_main\\.cpp:[0-9]+: condition",
            &assertFailure,
            Debugging,
        },
    };
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

    setEmergencyLogger (std::make_shared<MarkedLogger>());
    installEmergencyTerminateHandler();

    std::string_view requested = argc > 1 ? argv[1] : "";

    if (requested == "--list")
    {
        for (const auto& fatal_case : Fatal_Cases)
        {
            if (fatal_case.listed)
                std::cout << fatal_case.name << "\n";
        }
        return EXIT_SUCCESS;
    }

    for (const auto& fatal_case : Fatal_Cases)
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
