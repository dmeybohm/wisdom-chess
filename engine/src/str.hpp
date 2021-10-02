#ifndef WISDOM_STR_HPP
#define WISDOM_STR_HPP

#include "global.hpp"

namespace wisdom
{
    // "Chomp" the last newline of a string.
    [[nodiscard]] auto chomp (const std::string &str) -> std::string;

    // Split the string into a vector of strings.
    [[nodiscard]] auto split (const std::string &source,
                              const std::string &separator) ->
        std::vector<std::string>;

    // Join the vector of strings into a single string separated by separator.
    [[nodiscard]] auto join (const std::vector<std::string> &strings,
                             const std::string &separator) ->
        std::string;

    // Convert the string to an integer.
    [[nodiscard]] auto to_int (const std::string &str) -> int;
}

#endif //WISDOM_STR_HPP