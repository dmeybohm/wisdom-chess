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

    [[nodiscard]] auto
    getFirstHumanPlayerColor (const Players& players)
        -> std::optional<Color>;
}
