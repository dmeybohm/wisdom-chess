#pragma once

#include <optional>

#include "wisdom-chess/engine/game.hpp"

namespace wisdom::ui
{
    enum class DrawByRepetitionStatus
    {
        NotReached,
        Proposed,
        Accepted,
        Declined
    };

    // Frontend search depths count full moves; the engine counts plies.
    [[nodiscard]] constexpr auto
    fullMovesToPlyDepth (int full_moves)
        -> int
    {
        return full_moves * 2;
    }

    [[nodiscard]] auto
    getFirstHumanPlayerColor (const Players& players)
        -> std::optional<Color>;
}
