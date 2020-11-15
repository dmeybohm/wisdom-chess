#include <string>

std::string chomp (std::string str)
{
    std::string result { str };
    if (result.size() == 0)
        return result;
    if (result[result.size() - 1] == '\n')
        result = result.substr(0, result.size() - 1);
    return result;
}
