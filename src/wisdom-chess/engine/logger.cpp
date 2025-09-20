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

        void info (const string& output) const override
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
}
