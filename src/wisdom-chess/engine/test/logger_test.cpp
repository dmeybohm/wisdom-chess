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

namespace
{
    constexpr size_t Overhead = LogRingBuffer::Record_Overhead;

    // Bytes one line of the given text length occupies in the buffer.
    constexpr auto recordSize (size_t text_length) -> size_t
    {
        return Overhead + text_length;
    }
}

TEST_CASE( "LogRingBuffer" )
{
    SUBCASE( "starts empty with the requested capacity" )
    {
        LogRingBuffer buffer { 100 };

        CHECK( buffer.empty() );
        CHECK( buffer.count() == 0 );
        CHECK( buffer.sizeBytes() == 0 );
        CHECK( buffer.capacityBytes() == 100 );
        CHECK( buffer.entries().empty() );
    }

    SUBCASE( "defaults to 64 KB" )
    {
        LogRingBuffer buffer;

        CHECK( buffer.capacityBytes() == 64 * 1024 );
    }

    SUBCASE( "preserves order, levels and byte accounting" )
    {
        LogRingBuffer buffer { 100 };

        buffer.push (Logger::LogLevel_Info, "one");
        buffer.push (Logger::LogLevel_Debug, "two");
        buffer.push (Logger::LogLevel_Info, "three");

        auto entries = buffer.entries();
        REQUIRE( entries.size() == 3 );
        CHECK( buffer.count() == 3 );
        CHECK( buffer.sizeBytes() == recordSize (3) + recordSize (3) + recordSize (5) );
        CHECK( entries[0].text == "one" );
        CHECK( entries[0].level == Logger::LogLevel_Info );
        CHECK( entries[1].text == "two" );
        CHECK( entries[1].level == Logger::LogLevel_Debug );
        CHECK( entries[2].text == "three" );
        CHECK( entries[2].level == Logger::LogLevel_Info );
        CHECK( !buffer.empty() );
    }

    SUBCASE( "fills exactly to capacity without evicting" )
    {
        LogRingBuffer buffer { 2 * recordSize (4) };

        buffer.push (Logger::LogLevel_Info, "aaaa");
        buffer.push (Logger::LogLevel_Info, "bbbb");

        CHECK( buffer.count() == 2 );
        CHECK( buffer.sizeBytes() == buffer.capacityBytes() );
    }

    SUBCASE( "evicts the oldest whole line when full" )
    {
        LogRingBuffer buffer { 2 * recordSize (4) + 2 };

        buffer.push (Logger::LogLevel_Info, "aaaa");
        buffer.push (Logger::LogLevel_Info, "bbbb");
        buffer.push (Logger::LogLevel_Info, "cccc");

        auto entries = buffer.entries();
        REQUIRE( entries.size() == 2 );
        CHECK( buffer.sizeBytes() == 2 * recordSize (4) );
        CHECK( entries[0].text == "bbbb" );
        CHECK( entries[1].text == "cccc" );
    }

    SUBCASE( "evicts several lines for one large push" )
    {
        LogRingBuffer buffer { 3 * recordSize (3) };

        buffer.push (Logger::LogLevel_Info, "aaa");
        buffer.push (Logger::LogLevel_Info, "bbb");
        buffer.push (Logger::LogLevel_Info, "ccc");
        buffer.push (Logger::LogLevel_Info, "dddddddddddd");

        auto entries = buffer.entries();
        REQUIRE( entries.size() == 1 );
        CHECK( buffer.sizeBytes() == recordSize (12) );
        CHECK( entries[0].text == "dddddddddddd" );
    }

    SUBCASE( "truncates a line that cannot fit on its own" )
    {
        LogRingBuffer buffer { recordSize (5) };

        buffer.push (Logger::LogLevel_Info, "keep");
        buffer.push (Logger::LogLevel_Debug, "abcdefgh");

        auto entries = buffer.entries();
        REQUIRE( entries.size() == 1 );
        CHECK( buffer.sizeBytes() == buffer.capacityBytes() );
        CHECK( entries[0].text == "abcde" );
        CHECK( entries[0].level == Logger::LogLevel_Debug );
    }

    SUBCASE( "a line straddling the end of the storage reads back intact" )
    {
        // Two lines fill the buffer up to 4 bytes short of the end, so the
        // third line's header and text wrap around to the start.
        LogRingBuffer buffer { 2 * recordSize (6) + 4 };

        buffer.push (Logger::LogLevel_Info, "first!");
        buffer.push (Logger::LogLevel_Debug, "second");
        buffer.push (Logger::LogLevel_Info, "third!");

        auto entries = buffer.entries();
        REQUIRE( entries.size() == 2 );
        CHECK( entries[0].text == "second" );
        CHECK( entries[0].level == Logger::LogLevel_Debug );
        CHECK( entries[1].text == "third!" );
        CHECK( entries[1].level == Logger::LogLevel_Info );
    }

    SUBCASE( "keeps wrapping as lines keep arriving" )
    {
        LogRingBuffer buffer { 3 * recordSize (2) + 1 };

        for (int i = 0; i < 1000; i++)
            buffer.push (Logger::LogLevel_Info, std::to_string (i % 10) + "x");

        auto entries = buffer.entries();
        REQUIRE( entries.size() == 3 );
        CHECK( buffer.sizeBytes() == 3 * recordSize (2) );
        CHECK( entries[0].text == "7x" );
        CHECK( entries[1].text == "8x" );
        CHECK( entries[2].text == "9x" );
    }

    SUBCASE( "mixed line lengths survive many wraps" )
    {
        LogRingBuffer buffer { 64 };
        vector<string> pushed;

        for (int i = 0; i < 500; i++)
        {
            auto text = string (1 + (i * 7) % 20, static_cast<char> ('a' + i % 26));
            pushed.push_back (text);
            buffer.push (Logger::LogLevel_Info, text);
        }

        auto entries = buffer.entries();
        REQUIRE( !entries.empty() );
        CHECK( buffer.sizeBytes() <= buffer.capacityBytes() );

        // The retained lines are exactly the most recent ones, in order:
        auto first_kept = pushed.size() - entries.size();
        for (size_t i = 0; i < entries.size(); i++)
            CHECK( entries[i].text == pushed[first_kept + i] );
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
        CHECK( buffer.count() == 0 );
        CHECK( buffer.sizeBytes() == 0 );
    }

    SUBCASE( "drainTo on an empty buffer emits nothing" )
    {
        LogRingBuffer buffer { 100 };
        RecordingLogger sink;

        buffer.drainTo (sink);

        CHECK( sink.lines.empty() );
    }

    SUBCASE( "clear discards everything and the buffer is reusable" )
    {
        LogRingBuffer buffer { 100 };

        buffer.push (Logger::LogLevel_Info, "one");
        buffer.push (Logger::LogLevel_Info, "two");
        buffer.clear();

        CHECK( buffer.empty() );
        CHECK( buffer.sizeBytes() == 0 );

        buffer.push (Logger::LogLevel_Info, "three");
        auto entries = buffer.entries();
        REQUIRE( entries.size() == 1 );
        CHECK( entries[0].text == "three" );
        CHECK( buffer.sizeBytes() == recordSize (5) );
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
        // Each line is a 15 byte timestamp plus one character of text, so
        // this holds exactly two lines.
        BufferedLogger logger { sink, false, 2 * recordSize (16) };

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
