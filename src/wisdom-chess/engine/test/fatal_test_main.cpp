// Each case triggers one fatal error, so it has to run in its own process.
// CTest checks the output: see the "Fatal:" tests in CMakeLists.txt.

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/castling.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/move_list.hpp"

using namespace wisdom;

namespace
{
    struct MarkedLogger : Logger
    {
        void debug ([[maybe_unused]] const string& output) const override
        {
        }

        void info ([[maybe_unused]] const string& output) const override
        {
        }

        void emergency (const string& output) const override
        {
            std::cout << "[emergency] " << output << std::endl;
        }
    };

    // CTest fails any test that dies from a signal, whatever it printed, so
    // turn the abort into a marker and an ordinary exit.
    extern "C" void onAbort ([[maybe_unused]] int signal_number)
    {
        std::fputs ("[aborted]\n", stdout);
        std::fflush (stdout);
        std::_Exit (EXIT_FAILURE);
    }

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
        list.pop_back();
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
        code.setEnPassantTarget (Color::White, makeCoord (Black_En_Passant_Row, 0));
    }

    void uncaughtError()
    {
        throw Error { "boom", "extra detail" };
    }

    void expectsThroughNoexcept()
    {
        volatile bool condition = false;
        auto checked = [&]() noexcept
        {
            expects (condition);
        };
        checked();
    }
}

auto
main (int argc, char* argv[])
    -> int
{
#ifdef _MSC_VER
    // No dialog box or error report when the case aborts.
    _set_abort_behavior (0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    std::signal (SIGABRT, onAbort);

    setEmergencyLogger (std::make_shared<MarkedLogger>());
    installEmergencyTerminateHandler();

    std::string_view test_case = argc > 1 ? argv[1] : "";

    if (test_case == "append-overflow")
        appendOverflow();
    else if (test_case == "remove-from-empty")
        removeFromEmpty();
    else if (test_case == "bad-castling-flags")
        badCastlingFlags();
    else if (test_case == "bad-en-passant-row")
        badEnPassantRow();
    else if (test_case == "uncaught-error")
        uncaughtError();
    else if (test_case == "expects-through-noexcept")
        expectsThroughNoexcept();
    else
    {
        std::cout << "Unknown case: " << test_case << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "[survived] " << test_case << "\n";
    return EXIT_SUCCESS;
}
