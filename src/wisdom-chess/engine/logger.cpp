#include <cstdio>
#include <ctime>
#include <iostream>

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

    private:
        LogLevel my_log_level;

        static void write (const string& output)
        {
            auto result = output + "\n";
            std::cout << result;
        }
    };

    LogRingBuffer::LogRingBuffer (size_t capacity_bytes)
        : my_capacity_bytes { capacity_bytes }
    {}

    void LogRingBuffer::push (Logger::LogLevel level, string text)
    {
        if (text.size() > my_capacity_bytes)
            text.resize (my_capacity_bytes);

        while (!my_entries.empty() && my_size_bytes + text.size() > my_capacity_bytes)
        {
            my_size_bytes -= my_entries.front().text.size();
            my_entries.pop_front();
        }

        my_size_bytes += text.size();
        my_entries.push_back (LogEntry { level, std::move (text) });
    }

    void LogRingBuffer::drainTo (const Logger& sink)
    {
        for (const auto& entry : my_entries)
        {
            if (entry.level >= Logger::LogLevel_Debug)
                sink.debug (entry.text);
            else
                sink.info (entry.text);
        }

        clear();
    }

    void LogRingBuffer::clear()
    {
        my_entries.clear();
        my_size_bytes = 0;
    }

    auto
    LogRingBuffer::entries() const
        -> const std::deque<LogEntry>&
    {
        return my_entries;
    }

    auto
    LogRingBuffer::sizeBytes() const
        -> size_t
    {
        return my_size_bytes;
    }

    auto
    LogRingBuffer::capacityBytes() const
        -> size_t
    {
        return my_capacity_bytes;
    }

    auto
    LogRingBuffer::empty() const
        -> bool
    {
        return my_entries.empty();
    }

    auto
    formatLogTimestamp (chrono::system_clock::time_point time)
        -> string
    {
        auto whole_seconds = chrono::time_point_cast<chrono::seconds> (time);
        auto millis = chrono::duration_cast<chrono::milliseconds> (time - whole_seconds).count();
        std::time_t seconds_since_epoch = chrono::system_clock::to_time_t (whole_seconds);

        std::tm local_time {};
#ifdef _WIN32
        localtime_s (&local_time, &seconds_since_epoch);
#else
        localtime_r (&seconds_since_epoch, &local_time);
#endif

        char clock_text[16];
        std::strftime (clock_text, sizeof clock_text, "%H:%M:%S", &local_time);

        char result[32];
        std::snprintf (
            result,
            sizeof result,
            "[%s.%03lld] ",
            clock_text,
            static_cast<long long> (millis)
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
        std::lock_guard lock { my_mutex };

        if (enabled && !my_enabled)
            my_buffer.drainTo (*my_sink);

        my_enabled = enabled;
    }

    auto
    BufferedLogger::isEnabled() const
        -> bool
    {
        std::lock_guard lock { my_mutex };
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

    void BufferedLogger::log (LogLevel level, const string& output) const
    {
        auto line = formatLogTimestamp (chrono::system_clock::now()) + output;

        std::lock_guard lock { my_mutex };

        if (!my_enabled)
        {
            my_buffer.push (level, std::move (line));
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
}
