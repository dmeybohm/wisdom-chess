#ifndef WISDOMCHESS_GAME_SETTINGS_HPP
#define WISDOMCHESS_GAME_SETTINGS_HPP

#include "web_types.hpp"

namespace wisdom
{
    struct GameSettings
    {
        enum WebPlayer whitePlayer = Human;
        enum WebPlayer blackPlayer = ChessEngine;
        int thinkingTime = 2;
        int searchDepth = 3;
    };
};

#endif // WISDOMCHESS_GAME_SETTINGS_HPP
