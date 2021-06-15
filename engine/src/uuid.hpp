#ifndef WISDOM_UUID_HPP
#define WISDOM_UUID_HPP

#include "global.hpp"

namespace wisdom
{
    class Uuid
    {
    public:
        Uuid ();
        Uuid (const Uuid &) = default;
        Uuid (const std::string &str) : my_buf { str }
        {}

        std::string to_string() const;

        static Uuid Nil()
        {
            return Uuid { "00000000-0000-0000-0000-000000000000" };
        }

        bool operator== (const Uuid &other) const
        {
            return my_buf == other.my_buf;
        }

    private:
        std::string my_buf;
    };

}

#endif //WISDOM_UUID_HPP
