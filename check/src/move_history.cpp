#include "move_history.hpp"

#include <iostream>
#include <fstream>

namespace wisdom
{
    MoveHistory::MoveHistory (const MoveList &list)
    {
        this->my_moves = list;
    }

    void MoveHistory::save (const std::string &filename) const
    {
        std::ofstream file;
        file.open (filename);
        for (auto move : my_moves)
            file << to_string (move) << "\n";
        file.close ();
    }
}

