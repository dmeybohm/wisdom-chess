#ifndef WISDOM_UUID_HPP
#define WISDOM_UUID_HPP

#include "global.hpp"

namespace wisdom
{
    class Uuid
    {
    public:
        Uuid();
        Uuid(const Uuid &) = default;

        std::string to_string() const;

    private:
        std::string my_buf;
    };
}


#endif //WISDOM_UUID_HPP
