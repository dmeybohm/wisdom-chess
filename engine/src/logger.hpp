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

    Logger &make_null_logger ();
    Logger &make_standard_logger ();
}

#endif //WISDOM_LOGGER_HPP
