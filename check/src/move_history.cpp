#include "move_history.hpp"

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
            file << wisdom::to_string (move) << "\n";
        file.close ();
    }

    std::string MoveHistory::to_string () const
    {
        std::stringstream sstream;
        for (auto move : my_moves)
            sstream << wisdom::to_string (move) << "\n";
        return sstream.str();
    }
}

