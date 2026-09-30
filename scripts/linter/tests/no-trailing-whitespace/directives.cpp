#define MESSAGE R"(hello   
world)"
#define GREETING u8R"x(hi  
there)x"
#define WIDTH 80  
#define LONG(x) \
    ((x) + 1)
#define QUOTED "one   " \
    "two"  
#include <string>

namespace wisdom
{
    auto text() -> std::string_view
    {
        return MESSAGE;
    }
}
