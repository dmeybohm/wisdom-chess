#pragma once

#include <deque>
#include <mutex>

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    class Logger
    {
    public:
        enum LogLevel
        {
            LogLevel_Info = 0,
            LogLevel_Debug = 1,
        };

        // Put virtual destructor in the .cpp file to put vtable there:
        virtual ~Logger();

        virtual void debug (const string& output) const = 0;
        virtual void info (const string& output) const = 0;
    };

    // How much log output to retain while debug logging is switched off.
    inline constexpr size_t Default_Log_Buffer_Bytes = 64 * 1024;

    struct LogEntry
    {
        Logger::LogLevel level;
        string text;
    };

    // Retains the most recent log lines up to a byte budget. When the budget
    // is exceeded, the oldest whole entries are evicted first.
    class LogRingBuffer
    {
    public:
        explicit LogRingBuffer (size_t capacity_bytes = Default_Log_Buffer_Bytes);

        // Append an entry. An entry larger than the capacity is truncated.
        void push (Logger::LogLevel level, string text);

        // Replay every entry in order through the sink, then clear the buffer.
        void drainTo (const Logger& sink);

        void clear();

        [[nodiscard]] auto
        entries() const
            -> const std::deque<LogEntry>&;

        [[nodiscard]] auto
        sizeBytes() const
            -> size_t;

        [[nodiscard]] auto
        capacityBytes() const
            -> size_t;

        [[nodiscard]] auto
        empty() const
            -> bool;

    private:
        size_t my_capacity_bytes;
        size_t my_size_bytes = 0;
        std::deque<LogEntry> my_entries;
    };

    // Format a local-time prefix of the form "[HH:MM:SS.mmm] ".
    [[nodiscard]] auto
    formatLogTimestamp (chrono::system_clock::time_point time)
        -> string;

    // Wraps a sink logger. While disabled, timestamped lines are retained in
    // a ring buffer instead of being forwarded. Enabling replays the retained
    // lines into the sink and clears the buffer; later lines go straight
    // through.
    class BufferedLogger : public Logger
    {
    public:
        explicit BufferedLogger (
            shared_ptr<Logger> sink,
            bool enabled = false,
            size_t capacity_bytes = Default_Log_Buffer_Bytes
        );

        ~BufferedLogger() override;

        void setEnabled (bool enabled);

        [[nodiscard]] auto
        isEnabled() const
            -> bool;

        void debug (const string& output) const override;
        void info (const string& output) const override;

    private:
        shared_ptr<Logger> my_sink;
        mutable std::mutex my_mutex;
        mutable LogRingBuffer my_buffer;
        bool my_enabled;

        void log (LogLevel level, const string& output) const;
    };

    auto
    makeNullLogger()
        -> shared_ptr<Logger>;

    auto
    makeStandardLogger (Logger::LogLevel level = Logger::LogLevel_Debug)
        -> shared_ptr<Logger>;

    auto
    makeBufferedLogger (shared_ptr<Logger> sink, bool enabled = false)
        -> shared_ptr<BufferedLogger>;
}
