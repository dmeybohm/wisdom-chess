#pragma once

#include <chrono>

#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

namespace wisdom::ui
{
    // What every frontend lets the player configure, in the units its
    // controls use: the search depth in full moves and the thinking time
    // in seconds. The limits are the ranges those controls offer.
    struct GameSettings
    {
        static constexpr int Min_Search_Depth = 1;
        static constexpr int Max_Search_Depth = 8;
        static constexpr int Min_Thinking_Time = 1;
        static constexpr int Max_Thinking_Time = 30;

        static constexpr int Default_Search_Depth = Default_Max_Depth / 2;
        static constexpr int Default_Thinking_Time = Default_Max_Search_Seconds;

        wisdom::Players players { wisdom::Player::Human, wisdom::Player::ChessEngine };
        int searchDepth = Default_Search_Depth;
        int thinkingTime = Default_Thinking_Time;

        // Whether engine output is shown live. When off it is retained in a
        // buffer and replayed once switched on.
        bool debugLogging = false;

        [[nodiscard]] auto operator== (const GameSettings&) const -> bool = default;

        [[nodiscard]] constexpr auto
        searchDepthInPlies() const
            -> int
        {
            return fullMovesToPlyDepth (searchDepth);
        }

        [[nodiscard]] constexpr auto
        isInRange() const
            -> bool
        {
            return searchDepth >= Min_Search_Depth && searchDepth <= Max_Search_Depth
                && thinkingTime >= Min_Thinking_Time && thinkingTime <= Max_Thinking_Time;
        }

        // Configure the engine's game. Throws when a value is out of range.
        void applyTo (nonnull_observer_ptr<Game> game) const
        {
            expects (isInRange());
            game->setMaxDepth (searchDepthInPlies());
            game->setSearchTimeout (std::chrono::seconds { thinkingTime });
            game->setPlayers (players);
        }
    };
}
