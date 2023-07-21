#include "output_format.hpp"
#include "board.hpp"
#include "history.hpp"

#include <fstream>

namespace wisdom
{
    void FenOutputFormat::save (const string& filename, const Board& board,
                                [[maybe_unused]] const History& history, Color turn)
    {
        string output = board.toFenString (turn);

        std::ofstream file;
        file.open (filename);
        file << output << "\n";
        file.close();
    }

    void WisdomGameOutputFormat::save (const string& filename, [[maybe_unused]] const Board& board,
                                       const History& history, [[maybe_unused]] Color turn)
    {
        std::ofstream file;
        file.open (filename);
        for (auto move : history.getMoveHistory())
            file << wisdom::asString (move) << "\n";
        file.close();
    }
}
