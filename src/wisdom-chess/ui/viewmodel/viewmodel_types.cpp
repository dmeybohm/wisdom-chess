#include "wisdom-chess/ui/viewmodel/viewmodel_types.hpp"

namespace wisdom::ui
{
    auto
    getFirstHumanPlayerColor (const Players& players)
        -> std::optional<Color>
    {
        if (players[0] == Player::Human)
        {
            return Color::White;
        }
        if (players[1] == Player::Human)
        {
            return Color::Black;
        }

        return {};
    }
}
