#pragma once

#include "wisdom-chess/engine/game.hpp"

#include "wisdom-chess/ui/wasm/web_types.hpp"

namespace wisdom
{
    struct GameSettings
    {
        GameSettings() = default;

        GameSettings (
            int whitePlayer, 
            int blackPlayer, 
            int thinkingTime_, 
            int searchDepth_, 
            bool debugLogging_ = false
        )
            : whitePlayer { mapPlayer (mapPlayer (whitePlayer)) }
            , blackPlayer { mapPlayer (mapPlayer (blackPlayer)) }
            , thinkingTime { thinkingTime_ }
            , searchDepth { searchDepth_ }
            , debugLogging { debugLogging_ }
        {}

        GameSettings (
            enum WebPlayer whitePlayer_, 
            enum WebPlayer blackPlayer_, 
            int thinkingTime_, 
            int searchDepth_, 
            bool debugLogging_ = false
        )
            : whitePlayer { whitePlayer_ }
            , blackPlayer { blackPlayer_ }
            , thinkingTime { thinkingTime_ }
            , searchDepth { searchDepth_ }
            , debugLogging { debugLogging_ }
        {}

        enum WebPlayer whitePlayer = Human;
        enum WebPlayer blackPlayer = ChessEngine;

        int thinkingTime = Default_Max_Search_Seconds;

        int searchDepth = Default_Max_Depth / 2;

        // Whether engine output is shown live. When off it is retained in a
        // buffer and replayed once switched on.
        bool debugLogging = false;

        [[nodiscard]] static auto 
        mapHumanDepthToComputerDepth (int human_depth) 
            -> int
        {
            return human_depth * 2;
        }

        void applyToGame (observer_ptr<wisdom::Game> game) const
        {
            game->setSearchTimeout (std::chrono::seconds { thinkingTime });
            game->setMaxDepth (GameSettings::mapHumanDepthToComputerDepth (searchDepth));
            game->setPlayers ({ mapPlayer (whitePlayer), mapPlayer (blackPlayer) });
        }
    };
};

