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

}
