#pragma once

#include "wisdom-chess/engine/game.hpp"

#include "wisdom-chess/ui/wasm/web_types.hpp"
#include "wisdom-chess/ui/viewmodel/game_settings.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

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
            WebPlayer whitePlayer_, 
            WebPlayer blackPlayer_, 
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

        WebPlayer whitePlayer = WebPlayer::Human;
        WebPlayer blackPlayer = WebPlayer::ChessEngine;

        int thinkingTime = ui::GameSettings::Default_Thinking_Time;

        int searchDepth = ui::GameSettings::Default_Search_Depth;

        // Whether engine output is shown live. When off it is retained in a
        // buffer and replayed once switched on.
        bool debugLogging = false;

        // The same settings in the engine's terms.
        [[nodiscard]] auto
        toEngineSettings() const
            -> ui::GameSettings
        {
            return ui::GameSettings {
                .players = { mapPlayer (whitePlayer), mapPlayer (blackPlayer) },
                .searchDepth = searchDepth,
                .thinkingTime = thinkingTime,
                .debugLogging = debugLogging,
            };
        }

        void applyToGame (nonnull_observer_ptr<wisdom::Game> game) const
        {
            toEngineSettings().applyTo (game);
        }
    };
};

