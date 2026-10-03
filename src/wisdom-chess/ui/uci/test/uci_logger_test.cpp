#include <iostream>
#include <sstream>

#include <doctest/doctest.h>

#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/ui/uci/uci_interface.hpp"

using namespace wisdom;

namespace
{
    // Captures a stream for the lifetime of the object.
    struct CapturedStream
    {
        std::ostream& stream;
        std::ostringstream captured;
        std::streambuf* original; // lint-allow(raw-pointer): std::ios API

        explicit CapturedStream (std::ostream& stream_to_capture)
            : stream { stream_to_capture }
            , original { stream_to_capture.rdbuf (captured.rdbuf()) }
        {
        }

        ~CapturedStream()
        {
            stream.rdbuf (original);
        }
    };

    // Makes every write to std::cout throw, for the lifetime of the object.
    // std::cerr is tied to std::cout and flushes it first, so the tie is
    // undone too, or the failure would surface in std::cerr instead.
    struct FailingCout
    {
        struct RejectingBuffer : std::streambuf
        {
        };

        RejectingBuffer rejecting;
        std::streambuf* original_buffer = std::cout.rdbuf (&rejecting); // lint-allow(raw-pointer): std::ios API
        std::ios::iostate original_exceptions = std::cout.exceptions();
        std::ostream* original_tie = std::cerr.tie (nullptr); // lint-allow(raw-pointer): std::ios API

        FailingCout()
        {
            std::cout.exceptions (std::ios::badbit | std::ios::failbit);
        }

        ~FailingCout()
        {
            std::cout.exceptions (std::ios::goodbit);
            std::cout.clear();
            std::cout.rdbuf (original_buffer);
            std::cout.exceptions (original_exceptions);
            std::cerr.tie (original_tie);
        }
    };
}

TEST_CASE( "The UCI emergency logger writes each line as an info string" )
{
    CapturedStream cout { std::cout };
    auto logger = makeUciLogger (false);

    logger->emergency ("first\nsecond\n");

    CHECK( cout.captured.str() == "info string first\ninfo string second\n" );
}

TEST_CASE( "The UCI logger prefixes every line of a message" )
{
    CapturedStream cout { std::cout };
    auto logger = makeUciLogger (true);

    logger->debug ("first\nsecond");
    logger->info ("third\nfourth\n");

    CHECK( cout.captured.str() == "info string first\ninfo string second\ninfo third\ninfo fourth\n" );
}

TEST_CASE( "A failing standard output does not escape the UCI emergency logger" )
{
    CapturedStream cerr { std::cerr };
    auto logger = makeUciLogger (false);
    setEmergencyLogger (logger);

    {
        FailingCout failing_cout;
        CHECK_THROWS( std::cout << "proof that writes throw" << '\n' );

        // The override is noexcept, so an exception out of it would end the
        // process rather than fail an assertion.
        logger->emergency ("fatal");
        logEmergency ("fatal");
    }

    CHECK( cerr.captured.str() == "fatal\n" );

    // The failure must not leave the emergency path locked out.
    {
        CapturedStream cout { std::cout };
        logEmergency ("again");
        CHECK( cout.captured.str() == "info string again\n" );
    }

    setEmergencyLogger (nullptr);
}
