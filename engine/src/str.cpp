#include "str.hpp"

#include <iostream>
#include <sstream>

namespace wisdom
{
    using std::stringstream;

    auto chomp (const string& str) -> string
    {
        string result { str };

        if (result.empty ())
            return result;

        if (result[result.size () - 1] == '\n')
            result = result.substr (0, result.size () - 1);

        if (result[result.size () - 1] == '\r')
            result = result.substr (0, result.size () - 1);

        return result;
    }

    auto split (const string& source, const string& separator) -> vector<string>
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

    auto join (const vector<string>& strings, const string& separator) -> string
    {
        string result;

        for (auto& str : strings)
            result += str + separator;

        return result.substr (0, result.size () - separator.size ());
    }

    auto to_int (const string& str) -> int
    {
        stringstream ss { str };

        int x = 0;
        ss >> x;

        if (ss.fail ())
            throw Error { "Failed to convert" };

        return x;
    }
}
