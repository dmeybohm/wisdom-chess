#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/game.hpp"

namespace wisdom
{
    // Saves the game as FEN when the file name contains ".fen", and as
    // the engine's move list otherwise. Throws Error when the file
    // cannot be written.
    void saveGame (const Game& game, const string& filename);

    // Loads a game saved as a move list. Empty when the file cannot be
    // read or holds a move that does not parse.
    [[nodiscard]] auto
    loadGame (const string& filename, const Players& players)
        -> optional<Game>;
}
