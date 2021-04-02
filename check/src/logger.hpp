#ifndef WISDOM_LOGGER_HPP
#define WISDOM_LOGGER_HPP

#include "global.hpp"

namespace wisdom
{
    class Logger
    {
    public:
        virtual void println (const std::string &output) = 0;
    };

    class NullLogger : public Logger
    {
    public:
        NullLogger () = default;

        void println ([[maybe_unused]] const std::string &output) override
        {}
    };

    class StandardLogger : public Logger
    {
    private:
        std::mutex output_mutex;

    public:
        StandardLogger () = default;

        void println (const std::string &output) override
        {
            std::lock_guard lock { output_mutex };
            std::cout << output << '\n';
        }
    };
}

#endif //WISDOM_LOGGER_HPP
