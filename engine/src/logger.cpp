#include "logger.hpp"

#include <iostream>

namespace wisdom
{
    class NullLogger : public Logger
    {
    public:
        NullLogger () = default;
        ~NullLogger () override = default;

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

    auto make_null_logger () -> Logger&
    {
        static NullLogger null_logger {};
        return null_logger;
    }

    auto make_standard_logger (Logger::LogLevel level) -> Logger&
    {
        static StandardLogger standard_logger { level };
        return standard_logger;
    }
}
