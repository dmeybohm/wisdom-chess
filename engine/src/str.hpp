#ifndef WISDOM_STR_HPP
#define WISDOM_STR_HPP

#include "global.hpp"

namespace wisdom
{
    // "Chomp" the last newline of a string.
    auto chomp (const std::string &str) -> std::string;

    // Split the string into a vector of strings.
    auto split (const std::string &source, const std::string &separator) ->
        std::vector<std::string>;
}

#endif //WISDOM_STR_HPP