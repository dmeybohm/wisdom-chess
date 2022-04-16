#include "play.hpp"

#include <iostream>

using wisdom::Color;
using std::string;

int main (int argc, char **argv)
{
    try
    {
        auto human_player = Color::White;
        wisdom::play (human_player);
    }
    catch (const wisdom::Error &e)
    {
        std::cerr << "Uncaught Error!" << "\n";
        std::cerr << e.message() << "\n";
        std::cerr << e.extra_info() << "\n";
        std::terminate ();
    }

    return 0;
}
