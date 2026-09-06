#pragma once

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

    // Retains the most recent log lines in a single contiguous byte buffer
    // with wraparound. Each line is stored as a small header (level and
    // length) followed by its bytes. When the buffer is full, the oldest
    // whole lines are evicted first.
    class LogRingBuffer
    {
    public:
        // Bytes of header stored before each line's text.
        static constexpr size_t Record_Overhead = sizeof (uint8_t) + sizeof (uint32_t);

        explicit LogRingBuffer (size_t capacity_bytes = Default_Log_Buffer_Bytes);

        // Append a line. A line that cannot fit on its own is truncated.
        void push (Logger::LogLevel level, string_view text);

        // Replay every line in order through the sink, then clear the buffer.
        void drainTo (const Logger& sink);

        void clear();

        // Copy the retained lines out, oldest first.
        [[nodiscard]] auto
        entries() const
            -> vector<LogEntry>;

        // Number of retained lines.
        [[nodiscard]] auto
        count() const
            -> size_t;

        // Bytes in use, including the per-line headers.
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
        vector<char> my_storage;
        size_t my_head = 0;     // Offset of the oldest record.
        size_t my_used = 0;     // Bytes in use, headers included.
        size_t my_count = 0;

        struct Record
        {
            Logger::LogLevel level;
            size_t length;
        };

        [[nodiscard]] auto
        tailOffset() const
            -> size_t;

        [[nodiscard]] auto
        readRecord (size_t offset) const
            -> Record;

        [[nodiscard]] auto
        readText (size_t offset, size_t length) const
            -> string;

        void writeBytes (const char* source, size_t length);
        void popFront();

        template <typename Visitor>
        void forEachEntry (Visitor&& visit) const;
    };

    // Format a UTC time-of-day prefix of the form "[HH:MM:SS.mmm] ".
    // UTC is used because converting to local time needs either the
    // platform's localtime_r/localtime_s or a C++20 time zone database,
    // which the WebAssembly (libc++) builds do not provide.
    [[nodiscard]] auto
    formatLogTimestamp (chrono::system_clock::time_point time)
        -> string;

    // Wraps a sink logger. While disabled, timestamped lines are retained in
    // a ring buffer instead of being forwarded. Enabling replays the retained
    // lines into the sink and clears the buffer; later lines go straight
    // through.
    //
    // Not thread-safe: use one instance per thread.
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
