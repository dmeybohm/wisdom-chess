#include "move_history.hpp"

#include <iostream>
#include <fstream>

move_history_t::move_history_t (const move_history_t &other)
{
    this->my_moves = other.my_moves;
}

move_history_t::move_history_t (const move_list_t &list)
{
    this->my_moves = list;
}

void move_history_t::save (const std::string &filename)
{
    std::ofstream file;
    file.open(filename);
    for (auto move : my_moves)
        file << to_string(move) << "\n";
    file.close();
}

