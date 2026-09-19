#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>

#include "wisdom-chess/engine/logger.hpp"

namespace wisdom
{
    // Put destructor here to put vtable in this translation unit:
    Logger::~Logger() = default;

    class NullLogger : public Logger
    {
    public:
        NullLogger() = default;
        ~NullLogger() override = default;

        void debug ([[maybe_unused]] const string& output) const override
        {
        }

        void info ([[maybe_unused]] const string& output) const override
        {
        }

        void emergency ([[maybe_unused]] const string& output) const override
        {
        }
    };

    class StandardLogger : public Logger
    {
    public:
        explicit StandardLogger (LogLevel level)
            : my_log_level { level }
        {}

        ~StandardLogger() override = default;

        void debug (const string& output) const override
        {
            if (my_log_level >= LogLevel_Debug)
                write (output);
        }

        void info (const string& output) const override
        {
            if (my_log_level >= LogLevel_Info)
                write (output);
        }

        void emergency (const string& output) const override
        {
            std::cerr << output << '\n';
        }

    private:
        LogLevel my_log_level;

        static void write (const string& output)
        {
            auto result = output + "\n";
            std::cout << result;
        }
    };

    LogRingBuffer::LogRingBuffer (size_t capacity_bytes)
        : my_storage (std::max (capacity_bytes, Record_Overhead + 1))
    {}

    auto
    LogRingBuffer::tailOffset() const
        -> size_t
    {
        return (my_head + my_used) % my_storage.size();
    }

    void LogRingBuffer::writeBytes (const char* source, size_t length)
    {
        auto offset = tailOffset();
        auto until_end = std::min (length, my_storage.size() - offset);

        std::memcpy (&my_storage[offset], source, until_end);
        if (length > until_end)
            std::memcpy (&my_storage[0], source + until_end, length - until_end);

        my_used += length;
    }

    auto
    LogRingBuffer::readRecord (size_t offset) const
        -> Record
    {
        uint8_t level_byte = 0;
        uint32_t length = 0;
        char header[Record_Overhead];

        for (size_t i = 0; i < Record_Overhead; i++)
            header[i] = my_storage[(offset + i) % my_storage.size()];

        std::memcpy (&level_byte, header, sizeof level_byte);
        std::memcpy (&length, header + sizeof level_byte, sizeof length);

        return Record {
            static_cast<Logger::LogLevel> (level_byte),
            length
        };
    }

    auto
    LogRingBuffer::readText (size_t offset, size_t length) const
        -> string
    {
        auto text = string (length, '\0');
        auto until_end = std::min (length, my_storage.size() - offset);

        std::memcpy (text.data(), &my_storage[offset], until_end);
        if (length > until_end)
            std::memcpy (text.data() + until_end, &my_storage[0], length - until_end);

        return text;
    }

    void LogRingBuffer::popFront()
    {
        auto record = readRecord (my_head);
        auto record_size = Record_Overhead + record.length;

        my_head = (my_head + record_size) % my_storage.size();
        my_used -= record_size;
        my_count--;
    }

    void LogRingBuffer::push (Logger::LogLevel level, string_view text)
    {
        auto max_text = my_storage.size() - Record_Overhead;
        if (text.size() > max_text)
            text = text.substr (0, max_text);

        auto record_size = Record_Overhead + text.size();
        while (my_count > 0 && my_used + record_size > my_storage.size())
            popFront();

        auto level_byte = narrow<uint8_t> (static_cast<int> (level));
        auto length = narrow<uint32_t> (text.size());
        char header[Record_Overhead];
        std::memcpy (header, &level_byte, sizeof level_byte);
        std::memcpy (header + sizeof level_byte, &length, sizeof length);

        writeBytes (header, Record_Overhead);
        writeBytes (text.data(), text.size());
        my_count++;
    }

    template <typename Visitor>
    void LogRingBuffer::forEachEntry (Visitor&& visit) const
    {
        auto offset = my_head;

        for (size_t i = 0; i < my_count; i++)
        {
            auto record = readRecord (offset);
            auto text_offset = (offset + Record_Overhead) % my_storage.size();

            visit (record.level, readText (text_offset, record.length));
            offset = (text_offset + record.length) % my_storage.size();
        }
    }

    void LogRingBuffer::drainTo (const Logger& sink)
    {
        forEachEntry ([&sink] (Logger::LogLevel level, const string& text)
        {
            if (level >= Logger::LogLevel_Debug)
                sink.debug (text);
            else
                sink.info (text);
        });

        clear();
    }

    void LogRingBuffer::clear()
    {
        my_head = 0;
        my_used = 0;
        my_count = 0;
    }

    auto
    LogRingBuffer::entries() const
        -> vector<LogEntry>
    {
        vector<LogEntry> result;
        result.reserve (my_count);

        forEachEntry ([&result] (Logger::LogLevel level, string text)
        {
            result.push_back (LogEntry { level, std::move (text) });
        });

        return result;
    }

    auto
    LogRingBuffer::count() const
        -> size_t
    {
        return my_count;
    }

    auto
    LogRingBuffer::sizeBytes() const
        -> size_t
    {
        return my_used;
    }

    auto
    LogRingBuffer::capacityBytes() const
        -> size_t
    {
        return my_storage.size();
    }

    auto
    LogRingBuffer::empty() const
        -> bool
    {
        return my_count == 0;
    }

    auto
    formatLogTimestamp (chrono::system_clock::time_point time)
        -> string
    {
        using chrono::duration_cast;

        // system_clock counts from the Unix epoch in UTC, so the remainder
        // modulo one day is the UTC time of day. No time zone lookup needed.
        auto since_midnight =
            duration_cast<chrono::milliseconds> (time.time_since_epoch()) % chrono::hours { 24 };

        auto hours = duration_cast<chrono::hours> (since_midnight);
        auto minutes = duration_cast<chrono::minutes> (since_midnight - hours);
        auto seconds = duration_cast<chrono::seconds> (since_midnight - hours - minutes);
        auto millis = since_midnight - hours - minutes - seconds;

        char result[32];
        std::snprintf (
            result,
            sizeof result,
            "[%02d:%02d:%02d.%03d] ",
            narrow<int> (hours.count()),
            narrow<int> (minutes.count()),
            narrow<int> (seconds.count()),
            narrow<int> (millis.count())
        );
        return result;
    }

    BufferedLogger::BufferedLogger (
        shared_ptr<Logger> sink,
        bool enabled,
        size_t capacity_bytes
    )
        : my_sink { std::move (sink) }
        , my_buffer { capacity_bytes }
        , my_enabled { enabled }
    {}

    BufferedLogger::~BufferedLogger() = default;

    void BufferedLogger::setEnabled (bool enabled)
    {
        if (enabled && !my_enabled)
            my_buffer.drainTo (*my_sink);

        my_enabled = enabled;
    }

    auto
    BufferedLogger::isEnabled() const
        -> bool
    {
        return my_enabled;
    }

    void BufferedLogger::debug (const string& output) const
    {
        log (LogLevel_Debug, output);
    }

    void BufferedLogger::info (const string& output) const
    {
        log (LogLevel_Info, output);
    }

    void BufferedLogger::emergency (const string& output) const
    {
        string line;
        try
        {
            line = formatLogTimestamp (chrono::system_clock::now()) + output;
        }
        catch (...)
        {
            // Out of memory: the bare message is better than none.
            my_sink->emergency (output);
            return;
        }
        my_sink->emergency (line);
    }

    void BufferedLogger::log (LogLevel level, const string& output) const
    {
        auto line = formatLogTimestamp (chrono::system_clock::now()) + output;

        if (!my_enabled)
        {
            my_buffer.push (level, line);
            return;
        }

        if (level >= LogLevel_Debug)
            my_sink->debug (line);
        else
            my_sink->info (line);
    }

    auto
    makeNullLogger()
        -> shared_ptr<Logger>
    {
        return make_unique<NullLogger>();
    }

    auto
    makeStandardLogger (Logger::LogLevel level)
        -> shared_ptr<Logger>
    {
        return make_unique<StandardLogger> (level);
    }

    auto
    makeBufferedLogger (shared_ptr<Logger> sink, bool enabled)
        -> shared_ptr<BufferedLogger>
    {
        return make_shared<BufferedLogger> (std::move (sink), enabled);
    }

    namespace
    {
        std::mutex emergency_logger_mutex;
        shared_ptr<Logger> emergency_logger;
        std::atomic_flag emergency_in_progress;

        struct EmergencyInProgressRelease
        {
            ~EmergencyInProgressRelease()
            {
                emergency_in_progress.clear();
            }
        };

        auto
        describeCurrentException()
            -> string
        {
            auto current = std::current_exception();
            if (!current)
                return "Terminated without an active exception";

            try
            {
                std::rethrow_exception (current);
            }
            catch (const Error& e)
            {
                auto result = "Uncaught error: " + e.message();
                if (!e.extra_info().empty())
                    result += "\n" + e.extra_info();
                return result;
            }
            catch (const std::exception& e)
            {
                return string { "Uncaught exception: " } + e.what();
            }
            catch (...)
            {
                return "Uncaught unknown exception";
            }
        }

        [[noreturn]] void emergencyTerminateHandler() noexcept
        {
            try
            {
                logEmergency (describeCurrentException());
            }
            catch (...)
            {
                // Describing the exception failed, most likely for lack of memory.
                try
                {
                    std::cerr << "Terminating after an uncaught exception\n";
                }
                catch (...)
                {
                }
            }
            std::abort();
        }
    }

    void setEmergencyLogger (shared_ptr<Logger> logger)
    {
        // Swap, so the previous logger is destroyed outside the lock.
        std::lock_guard lock { emergency_logger_mutex };
        emergency_logger.swap (logger);
    }

    void logEmergency (const string& message) noexcept
    {
        // Each sink gets its own attempt, so a failure in one cannot cost the other.
        try
        {
            std::cerr << message << '\n';
        }
        catch (...)
        {
        }

        try
        {
            // A logger that fails while reporting must not recurse into itself.
            if (emergency_in_progress.test_and_set())
                return;
            EmergencyInProgressRelease release;

            shared_ptr<Logger> logger;
            {
                std::lock_guard lock { emergency_logger_mutex };
                logger = emergency_logger;
            }

            if (logger)
                logger->emergency (message);
        }
        catch (...)
        {
        }
    }

    void installEmergencyTerminateHandler()
    {
        std::set_terminate (emergencyTerminateHandler);
    }
}
