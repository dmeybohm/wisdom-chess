#include "play.hpp"

#include <iostream>
#include <argh.h>

using wisdom::Color;
using std::string;

int main (int argc, char **argv)
{
    argh::parser cmdline { argv, argh::parser::PREFER_FLAG_FOR_UNREG_OPTION };

    Color human_player = Color::White;

    if (cmdline("--player"))
    {
        string player_str = cmdline("--player").str();

        if (player_str == "White")
            human_player = Color::White;
        else if (player_str == "Black")
            human_player = Color::Black;
        else
        {
            std::cerr << "Invalid player" << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Overriding player to " << wisdom::to_string (human_player) << "\n";
    }

    try
    {
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
