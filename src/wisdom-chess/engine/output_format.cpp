#include <fstream>

#include "wisdom-chess/engine/output_format.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/history.hpp"

namespace wisdom
{
    // Put destructor here to put vtable in this translation unit:
    OutputFormat::~OutputFormat() = default;

    void 
    FenOutputFormat::save (
        const string& filename, 
        const Board& board,
        [[maybe_unused]] const History& history, 
        Color turn
    ) {
        string output = board.toFenString (turn);

        std::ofstream file { filename };
        if (!file)
            throw Error { "Cannot open " + filename + " for writing." };

        file << output << "\n";
        file.close();
        if (!file)
            throw Error { "Error writing " + filename + "." };
    }

    void 
    WisdomGameOutputFormat::save (
        const string& filename, 
        [[maybe_unused]] const Board& board,
        const History& history, 
        [[maybe_unused]] Color turn
    ) {
        std::ofstream file { filename };
        if (!file)
            throw Error { "Cannot open " + filename + " for writing." };

        for (auto move : history.getMoveHistory())
            file << wisdom::asString (move) << "\n";
        file.close();
        if (!file)
            throw Error { "Error writing " + filename + "." };
    }
}
