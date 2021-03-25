#ifndef WISDOM_STR_HPP
#define WISDOM_STR_HPP

#include <string>

namespace wisdom
{
    std::string chomp (const std::string &str);

    bool cstr_equals (const char *str1, const char *str2);
}

#endif //WISDOM_STR_HPP