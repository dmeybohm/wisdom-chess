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

        explicit Uuid (int64_t rand64) : my_rand64 { rand64 }
        {}


        [[nodiscard]] auto to_string() const -> string;

        static Uuid Nil()
        {
            return Uuid { 0 };
        }

        bool operator== (const Uuid &other) const
        {
            return my_rand64 == other.my_rand64;
        }

    private:
        int64_t my_rand64;
    };
}

#endif //WISDOM_UUID_HPP
