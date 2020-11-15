
#include "move_history.hpp"

#include <iostream>
#include <fstream>

void move_history_t::save(std::string filename)
{
    std::ofstream file;
    file.open(filename);
    for (auto move : my_moves)
        file << to_string(move) << "\n";
    file.close();
}