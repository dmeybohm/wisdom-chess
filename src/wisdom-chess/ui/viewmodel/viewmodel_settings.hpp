#pragma once

#include <chrono>

#include "wisdom-chess/engine/game.hpp"

namespace wisdom::ui
{
    struct ViewModelSettings
    {
        wisdom::Player whitePlayer = wisdom::Player::Human;
        wisdom::Player blackPlayer = wisdom::Player::ChessEngine;
        int searchDepth = Default_Max_Depth / 2;
        int thinkingTime = Default_Max_Search_Seconds;

        ViewModelSettings() = default;

        ViewModelSettings (
            wisdom::Player white,
            wisdom::Player black,
            int depth,
            int time
        )
            : whitePlayer { white }
            , blackPlayer { black }
            , searchDepth { depth }
            , thinkingTime { time }
        {}

        [[nodiscard]] auto
        internalDepth() const
            -> int
        {
            return searchDepth * 2;
        }

        void applyToGame (observer_ptr<Game> game) const
        {
            game->setSearchTimeout (std::chrono::seconds { thinkingTime });
            game->setMaxDepth (internalDepth());
            game->setPlayers ({ whitePlayer, blackPlayer });
        }

        friend auto
        operator== (const ViewModelSettings& a, const ViewModelSettings& b)
            -> bool = default;

        friend auto
        operator!= (const ViewModelSettings& a, const ViewModelSettings& b)
            -> bool = default;
    };
}
