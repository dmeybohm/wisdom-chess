
#ifndef WIZDUMB_OUTPUT_HPP
#define WIZDUMB_OUTPUT_HPP

#include <iostream>
#include <mutex>

namespace wisdom
{
    class Output
    {
    public:
        virtual void println (const std::string &output) = 0;
    };

    class NullOutput : public Output
    {
    public:
        NullOutput () = default;

        void println ([[maybe_unused]] const std::string &output) override
        {}
    };

    class StandardOutput : public Output
    {
    private:
        std::mutex output_mutex;

    public:
        StandardOutput () = default;

        void println (const std::string &output) override
        {
            std::lock_guard lock { output_mutex };
            std::cout << output << '\n';
        }
    };

}
#endif //WIZDUMB_OUTPUT_HPP
