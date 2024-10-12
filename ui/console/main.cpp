#include <iostream>

#include "wisdom-chess/engine/global.hpp"

#include "play.hpp"

using std::string;

int main()
{
    try
    {
        wisdom::ui::console::play();
    }
    catch (const wisdom::Error& e)
    {
        std::cerr << "Uncaught Error!" << "\n";
        std::cerr << e.message() << "\n";
        std::cerr << e.extra_info() << "\n";
        std::terminate();
    }

    return 0;
}
