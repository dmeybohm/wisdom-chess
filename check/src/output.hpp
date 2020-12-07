
#ifndef WIZDUMB_OUTPUT_HPP
#define WIZDUMB_OUTPUT_HPP

#include <iostream>

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

    public:
        standard_output () : out { std::cout }
        {}

        void println (const std::string &output) override
        {
            std::cout << output << '\n';
        }
    };

}
#endif //WIZDUMB_OUTPUT_HPP
