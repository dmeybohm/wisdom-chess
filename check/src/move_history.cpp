
#include "move_history.hpp"

#include <iostream>
#include <fstream>

move_history_t::move_history_t(const move_history_t &other)
{
    this->my_moves = other.my_moves;
}

void move_history_t::save(std::string filename)
{
    std::ofstream file;
    file.open(filename);
    for (auto move : my_moves)
        file << to_string(move) << "\n";
    file.close();
}