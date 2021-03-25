#include <string>

namespace wisdom
{
    std::string chomp (const std::string &str)
    {
        std::string result { str };
        if (result.empty ())
            return result;
        if (result[result.size () - 1] == '\n')
            result = result.substr (0, result.size () - 1);
        return result;
    }

    bool cstr_equals (const char *str1, const char *str2)
    {
        std::string s1 { str1 };
        std::string s2 { str2 };
        return s1 == s2;
    }

}
