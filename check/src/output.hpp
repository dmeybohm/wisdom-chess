
#ifndef WIZDUMB_OUTPUT_HPP
#define WIZDUMB_OUTPUT_HPP

#include <iostream>
#include <mutex>

namespace wisdom
{
    class output
    {
    public:
        virtual void println (const std::string &output) = 0;
    };

    class null_output : public output
    {
    public:
        null_output () = default;

        void println (const std::string &output) override
        {}
    };

    class standard_output : public output
    {
    private:
        const std::ostream &out;
        std::mutex output_mutex;

    public:
        standard_output () : out { std::cout }
        {}

        void println (const std::string &output) override
        {
            std::lock_guard lock { output_mutex };
            std::cout << output << '\n';
        }
    };

}
#endif //WIZDUMB_OUTPUT_HPP
