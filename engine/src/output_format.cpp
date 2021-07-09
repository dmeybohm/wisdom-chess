#include "board.hpp"
#include "output_format.hpp"
#include "history.hpp"

#include <fstream>

namespace wisdom
{
    void FenOutputFormat::save (const std::string &filename, const Board &board,
                                [[maybe_unused]] const History &history, Color turn)
    {
        std::string output = board.to_fen_string (turn);

        std::ofstream file;
        file.open (filename);
        file << output << "\n";
        file.close ();
    }


    void WisdomGameOutputFormat::save (const std::string &filename,
                                       [[maybe_unused]] const Board &board, const History &history,
                                       [[maybe_unused]] Color turn)
    {
        std::ofstream file;
        file.open (filename);
        for (auto move : history.get_move_history())
            file << wisdom::to_string (move) << "\n";
        file.close ();
    }
}
