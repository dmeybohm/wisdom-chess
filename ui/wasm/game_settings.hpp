#ifndef WISDOMCHESS_GAME_SETTINGS_HPP
#define WISDOMCHESS_GAME_SETTINGS_HPP

#include "web_types.hpp"

namespace wisdom
{
    struct GameSettings
    {
        enum WebPlayer whitePlayer = Human;
        enum WebPlayer blackPlayer = ChessEngine;
#if NDEBUG
        int thinkingTime = 2;
#else
        int thinkingTime = 4;
#endif

        int searchDepth = 3;

        [[nodiscard]] static auto map_human_depth_to_computer_depth (int human_depth) -> int
        {
            return human_depth * 2 - 1;
        }
    };
};

#endif // WISDOMCHESS_GAME_SETTINGS_HPP
