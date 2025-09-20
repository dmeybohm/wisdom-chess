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

    auto
    makeNullLogger()
        -> shared_ptr<Logger>;

    auto
    makeStandardLogger (Logger::LogLevel level = Logger::LogLevel_Debug)
        -> shared_ptr<Logger>;
}
