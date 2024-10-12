#pragma once

#include "wisdom-chess/engine/global.hpp"

namespace wisdom
{
    // "Chomp" the last newline of a string.
    [[nodiscard]] auto 
    chomp (const string& str) 
        -> string;

    // Split the string into a vector of strings.
    [[nodiscard]] auto 
    split (const string& source, const string& separator) 
        -> vector<string>;

    // Join the vector of strings into a single string separated by separator.
    [[nodiscard]] auto 
    join (const vector<string>& strings, const string& separator) 
        -> string;

    // Convert the string to an integer.
    [[nodiscard]] auto 
    toInt (const string& str) 
        -> optional<int>;
}
