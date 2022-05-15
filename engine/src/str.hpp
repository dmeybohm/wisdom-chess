#ifndef WISDOM_CHESS_STR_HPP
#define WISDOM_CHESS_STR_HPP

#include "global.hpp"

namespace wisdom
{
    // "Chomp" the last newline of a string.
    [[nodiscard]] auto chomp (const string& str) -> string;

    // Split the string into a vector of strings.
    [[nodiscard]] auto split (const string& source, const string& separator) -> vector<string>;

    // Join the vector of strings into a single string separated by separator.
    [[nodiscard]] auto join (const vector<string>& strings, const string& separator) -> string;

    // Convert the string to an integer.
    [[nodiscard]] auto to_int (const string& str) -> int;
}

#endif // WISDOM_CHESS_STR_HPP
