#ifndef WISDOMCHESS_GAME_SETTINGS_HPP
#define WISDOMCHESS_GAME_SETTINGS_HPP

#include "web_types.hpp"
#include "game.hpp"

namespace wisdom
{
    struct GameSettings
    {
        GameSettings() = default;

        GameSettings (int whitePlayer, int blackPlayer, int thinkingTime_, int searchDepth_)
            : whitePlayer { mapPlayer (mapPlayer (whitePlayer)) }
            , blackPlayer { mapPlayer (mapPlayer (blackPlayer)) }
            , thinkingTime { thinkingTime_ }
            , searchDepth { searchDepth_ }
        {}

        GameSettings (enum WebPlayer whitePlayer_, enum WebPlayer blackPlayer_, int thinkingTime_, int searchDepth_)
            : whitePlayer { whitePlayer_ }
            , blackPlayer { blackPlayer_ }
            , thinkingTime { thinkingTime_ }
            , searchDepth { searchDepth_ }
        {}

        enum WebPlayer whitePlayer = Human;
        enum WebPlayer blackPlayer = ChessEngine;

#if NDEBUG
        int thinkingTime = 2;
#else
        int thinkingTime = 4;
#endif

        int searchDepth = 3;


        [[nodiscard]] static auto mapHumanDepthToComputerDepth (int human_depth) -> int
        {
            return human_depth * 2 - 1;
        }

        void applyToGame (observer_ptr<wisdom::Game> game) const
        {
            game->setSearchTimeout (std::chrono::seconds { thinkingTime });
            game->setMaxDepth (GameSettings::mapHumanDepthToComputerDepth (searchDepth));
            game->setPlayers ({ mapPlayer (whitePlayer), mapPlayer (blackPlayer) });
        }
    };
};

#endif // WISDOMCHESS_GAME_SETTINGS_HPP
