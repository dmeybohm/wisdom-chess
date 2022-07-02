#ifndef WISDOM_LOGGER_HPP
#define WISDOM_LOGGER_HPP

#include "global.hpp"

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

        virtual ~Logger () = default;

        virtual void debug (const string& output) const = 0;
        virtual void info (const string& output) const = 0;
    };

    auto make_null_logger () -> Logger&;
    auto make_standard_logger (Logger::LogLevel level = Logger::LogLevel_Debug) -> Logger&;
}

#endif // WISDOM_LOGGER_HPP
