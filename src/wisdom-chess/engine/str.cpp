#include <iostream>
#include <sstream>

#include "wisdom-chess/engine/str.hpp"

namespace wisdom
{
    using std::stringstream;

    auto chomp (const string& str) -> string
    {
        string result { str };

        while (!result.empty() && isspace (result.back()))
            result = result.substr (0, result.size() - 1);

        return result;
    }

    auto split (
        const string& source, 
        const string& separator
    ) 
        -> vector<string>
    {
        vector<string> result;
        string::size_type offset = 0;
        string::size_type next_offset;

        while ((next_offset = source.find_first_of (separator, offset)) != string::npos)
        {
            result.push_back (source.substr (offset, next_offset - offset));
            offset = next_offset + 1;
        }

        // push last:
        result.push_back (source.substr (offset));
        return result;
    }

    auto join (
        const vector<string>& strings, 
        const string& separator
    ) 
        -> string
    {
        string result;

        for (const auto& str : strings)
            result += str + separator;

        return result.substr (
            0,
            result.size() > separator.size() ? result.size() - separator.size() : 0
        );
    }

    auto 
    toInt (const string& str) 
        -> optional<int>
    {
        stringstream ss { str };

        int x = 0;
        ss >> x;

        if (ss.fail())
            return nullopt;

        return x;
    }
}
