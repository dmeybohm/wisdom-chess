#include "logger.hpp"

#include <iostream>
#include <mutex>

namespace wisdom
{
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
            std::cout.flush();
        }
    };

    Logger &make_null_logger()
    {
        static NullLogger null_logger {};
        return null_logger;
    }

    Logger &make_standard_logger ()
    {
        static StandardLogger standard_logger {};
        return standard_logger;
    }
}
