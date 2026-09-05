#include <regex>

#include "wisdom-chess/engine/logger.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;
using std::string;
using std::vector;

namespace
{
    struct RecordingLogger : Logger
    {
        mutable vector<LogEntry> lines;

        void debug (const string& output) const override
        {
            lines.push_back (LogEntry { LogLevel_Debug, output });
        }

        void info (const string& output) const override
        {
            lines.push_back (LogEntry { LogLevel_Info, output });
        }
    };

    const std::regex Timestamp_Prefix { R"(^\[\d{2}:\d{2}:\d{2}\.\d{3}\] )" };

    auto hasTimestamp (const string& line) -> bool
    {
        return std::regex_search (line, Timestamp_Prefix);
    }

    // Strip the "[HH:MM:SS.mmm] " prefix.
    auto withoutTimestamp (const string& line) -> string
    {
        return std::regex_replace (line, Timestamp_Prefix, "");
    }
}

TEST_CASE( "LogRingBuffer" )
{
    SUBCASE( "starts empty with the requested capacity" )
    {
        LogRingBuffer buffer { 10 };

        CHECK( buffer.empty() );
        CHECK( buffer.sizeBytes() == 0 );
        CHECK( buffer.capacityBytes() == 10 );
        CHECK( buffer.entries().empty() );
    }

    SUBCASE( "defaults to 64 KB" )
    {
        LogRingBuffer buffer;

        CHECK( buffer.capacityBytes() == 64 * 1024 );
    }

    SUBCASE( "preserves order and counts bytes" )
    {
        LogRingBuffer buffer { 100 };

        buffer.push (Logger::LogLevel_Info, "one");
        buffer.push (Logger::LogLevel_Debug, "two");
        buffer.push (Logger::LogLevel_Info, "three");

        REQUIRE( buffer.entries().size() == 3 );
        CHECK( buffer.sizeBytes() == 11 );
        CHECK( buffer.entries()[0].text == "one" );
        CHECK( buffer.entries()[0].level == Logger::LogLevel_Info );
        CHECK( buffer.entries()[1].text == "two" );
        CHECK( buffer.entries()[1].level == Logger::LogLevel_Debug );
        CHECK( buffer.entries()[2].text == "three" );
        CHECK( !buffer.empty() );
    }

    SUBCASE( "fills exactly to capacity without evicting" )
    {
        LogRingBuffer buffer { 8 };

        buffer.push (Logger::LogLevel_Info, "aaaa");
        buffer.push (Logger::LogLevel_Info, "bbbb");

        REQUIRE( buffer.entries().size() == 2 );
        CHECK( buffer.sizeBytes() == 8 );
    }

    SUBCASE( "evicts the oldest whole entry when full" )
    {
        LogRingBuffer buffer { 10 };

        buffer.push (Logger::LogLevel_Info, "aaaa");
        buffer.push (Logger::LogLevel_Info, "bbbb");
        buffer.push (Logger::LogLevel_Info, "cccc");

        REQUIRE( buffer.entries().size() == 2 );
        CHECK( buffer.sizeBytes() == 8 );
        CHECK( buffer.entries()[0].text == "bbbb" );
        CHECK( buffer.entries()[1].text == "cccc" );
    }

    SUBCASE( "evicts several entries for one large push" )
    {
        LogRingBuffer buffer { 10 };

        buffer.push (Logger::LogLevel_Info, "aaa");
        buffer.push (Logger::LogLevel_Info, "bbb");
        buffer.push (Logger::LogLevel_Info, "ccc");
        buffer.push (Logger::LogLevel_Info, "dddddddd");

        REQUIRE( buffer.entries().size() == 1 );
        CHECK( buffer.sizeBytes() == 8 );
        CHECK( buffer.entries()[0].text == "dddddddd" );
    }

    SUBCASE( "truncates an entry larger than the capacity" )
    {
        LogRingBuffer buffer { 5 };

        buffer.push (Logger::LogLevel_Info, "keep me");
        buffer.push (Logger::LogLevel_Debug, "abcdefgh");

        REQUIRE( buffer.entries().size() == 1 );
        CHECK( buffer.sizeBytes() == 5 );
        CHECK( buffer.entries()[0].text == "abcde" );
        CHECK( buffer.entries()[0].level == Logger::LogLevel_Debug );
    }

    SUBCASE( "keeps wrapping as entries keep arriving" )
    {
        LogRingBuffer buffer { 6 };

        for (int i = 0; i < 100; i++)
            buffer.push (Logger::LogLevel_Info, std::to_string (i % 10) + "x");

        REQUIRE( buffer.entries().size() == 3 );
        CHECK( buffer.sizeBytes() == 6 );
        CHECK( buffer.entries()[0].text == "7x" );
        CHECK( buffer.entries()[1].text == "8x" );
        CHECK( buffer.entries()[2].text == "9x" );
    }

    SUBCASE( "drainTo replays in order with levels and then clears" )
    {
        LogRingBuffer buffer { 100 };
        RecordingLogger sink;

        buffer.push (Logger::LogLevel_Info, "first");
        buffer.push (Logger::LogLevel_Debug, "second");
        buffer.push (Logger::LogLevel_Info, "third");

        buffer.drainTo (sink);

        REQUIRE( sink.lines.size() == 3 );
        CHECK( sink.lines[0].level == Logger::LogLevel_Info );
        CHECK( sink.lines[0].text == "first" );
        CHECK( sink.lines[1].level == Logger::LogLevel_Debug );
        CHECK( sink.lines[1].text == "second" );
        CHECK( sink.lines[2].level == Logger::LogLevel_Info );
        CHECK( sink.lines[2].text == "third" );

        CHECK( buffer.empty() );
        CHECK( buffer.sizeBytes() == 0 );
    }

    SUBCASE( "drainTo on an empty buffer emits nothing" )
    {
        LogRingBuffer buffer { 100 };
        RecordingLogger sink;

        buffer.drainTo (sink);

        CHECK( sink.lines.empty() );
    }

    SUBCASE( "clear discards everything" )
    {
        LogRingBuffer buffer { 100 };

        buffer.push (Logger::LogLevel_Info, "one");
        buffer.push (Logger::LogLevel_Info, "two");
        buffer.clear();

        CHECK( buffer.empty() );
        CHECK( buffer.sizeBytes() == 0 );

        buffer.push (Logger::LogLevel_Info, "three");
        REQUIRE( buffer.entries().size() == 1 );
        CHECK( buffer.sizeBytes() == 5 );
    }
}

TEST_CASE( "formatLogTimestamp" )
{
    SUBCASE( "has the [HH:MM:SS.mmm] shape" )
    {
        auto now = chrono::system_clock::now();
        auto stamp = formatLogTimestamp (now);

        CHECK( hasTimestamp (stamp) );
        CHECK( stamp.size() == 15 );
    }

    SUBCASE( "carries the milliseconds" )
    {
        auto seconds = chrono::time_point_cast<chrono::seconds> (chrono::system_clock::now());
        auto stamp = formatLogTimestamp (seconds + chrono::milliseconds { 42 });

        CHECK( hasTimestamp (stamp) );
        CHECK( stamp.substr (9) == ".042] " );
    }
}

TEST_CASE( "BufferedLogger" )
{
    auto sink = std::make_shared<RecordingLogger>();

    SUBCASE( "starts disabled and buffers instead of forwarding" )
    {
        auto logger = makeBufferedLogger (sink);

        CHECK( !logger->isEnabled() );

        logger->info ("hidden info");
        logger->debug ("hidden debug");

        CHECK( sink->lines.empty() );
    }

    SUBCASE( "enabling replays the buffer in order and then forwards live" )
    {
        auto logger = makeBufferedLogger (sink);

        logger->info ("first");
        logger->debug ("second");

        logger->setEnabled (true);

        CHECK( logger->isEnabled() );
        REQUIRE( sink->lines.size() == 2 );
        CHECK( sink->lines[0].level == Logger::LogLevel_Info );
        CHECK( withoutTimestamp (sink->lines[0].text) == "first" );
        CHECK( sink->lines[1].level == Logger::LogLevel_Debug );
        CHECK( withoutTimestamp (sink->lines[1].text) == "second" );

        logger->debug ("third");

        REQUIRE( sink->lines.size() == 3 );
        CHECK( sink->lines[2].level == Logger::LogLevel_Debug );
        CHECK( withoutTimestamp (sink->lines[2].text) == "third" );
    }

    SUBCASE( "the buffer is empty after enabling" )
    {
        auto logger = makeBufferedLogger (sink);

        logger->info ("before");
        logger->setEnabled (true);
        REQUIRE( sink->lines.size() == 1 );

        // Re-enabling must not replay anything a second time:
        logger->setEnabled (true);
        CHECK( sink->lines.size() == 1 );

        // Nor should a disable/enable cycle with nothing logged in between:
        logger->setEnabled (false);
        logger->setEnabled (true);
        CHECK( sink->lines.size() == 1 );
    }

    SUBCASE( "disabling again resumes buffering" )
    {
        auto logger = makeBufferedLogger (sink, true);

        logger->info ("live");
        REQUIRE( sink->lines.size() == 1 );

        logger->setEnabled (false);
        logger->info ("held");
        CHECK( sink->lines.size() == 1 );

        logger->setEnabled (true);
        REQUIRE( sink->lines.size() == 2 );
        CHECK( withoutTimestamp (sink->lines[1].text) == "held" );
    }

    SUBCASE( "starting enabled forwards immediately" )
    {
        auto logger = makeBufferedLogger (sink, true);

        CHECK( logger->isEnabled() );

        logger->debug ("now");

        REQUIRE( sink->lines.size() == 1 );
        CHECK( withoutTimestamp (sink->lines[0].text) == "now" );
    }

    SUBCASE( "honors the capacity while buffering" )
    {
        // Each line is a 15 byte timestamp plus one character, so 40 bytes
        // holds two lines.
        BufferedLogger logger { sink, false, 40 };

        logger.info ("1");
        logger.info ("2");
        logger.info ("3");

        logger.setEnabled (true);

        REQUIRE( sink->lines.size() == 2 );
        CHECK( withoutTimestamp (sink->lines[0].text) == "2" );
        CHECK( withoutTimestamp (sink->lines[1].text) == "3" );
    }

    SUBCASE( "every forwarded line carries a timestamp" )
    {
        auto logger = makeBufferedLogger (sink);

        logger->info ("buffered");
        logger->setEnabled (true);
        logger->debug ("live");

        REQUIRE( sink->lines.size() == 2 );
        CHECK( hasTimestamp (sink->lines[0].text) );
        CHECK( hasTimestamp (sink->lines[1].text) );
    }
}
