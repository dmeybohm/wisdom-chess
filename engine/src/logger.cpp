#include "logger.hpp"

#include <iostream>
#include <mutex>

namespace wisdom
{
    class NullLogger : public Logger
    {
    public:
        NullLogger () = default;

        void println ([[maybe_unused]] const string &output) const override
        {}
    };

    class StandardLogger : public Logger
    {
    private:
        mutable std::mutex output_mutex;

    public:
        StandardLogger () = default;

        void println (const string &output) const override
        {
            std::lock_guard lock { output_mutex };
            std::cout << output << '\n';
            std::cout.flush();
        }
    };

    auto make_null_logger() -> Logger&
    {
        static NullLogger null_logger {};
        return null_logger;
    }

    auto make_standard_logger () -> Logger&
    {
        static StandardLogger standard_logger {};
        return standard_logger;
    }
}
