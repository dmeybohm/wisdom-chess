#ifndef WISDOM_LOGGER_HPP
#define WISDOM_LOGGER_HPP

#include "global.hpp"

namespace wisdom
{
    class Logger
    {
    public:
        virtual void println (const string &output) const = 0;
    };

    auto make_null_logger () -> Logger&;
    auto make_standard_logger () -> Logger&;
}

#endif //WISDOM_LOGGER_HPP
