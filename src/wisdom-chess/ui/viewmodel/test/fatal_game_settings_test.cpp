#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/test/fatal_test.hpp"
#include "wisdom-chess/ui/viewmodel/game_settings.hpp"

using namespace wisdom;
using wisdom::ui::GameSettings;

namespace
{
    // The engine allows deeper searches and longer timeouts than these
    // limits, so only GameSettings::isInRange() rejects the values above
    // them.
    void
    applySettings (int search_depth, int thinking_time)
    {
        auto game = Game::createStandardGame();
        int volatile depth = search_depth;
        int volatile time = thinking_time;
        GameSettings { .searchDepth = depth, .thinkingTime = time }.applyTo (&game);
    }

    FATAL_CASE(
        "game-settings-search-depth-below-range",
        "Precondition failed at .*game_settings\\.hpp:[0-9]+: isInRange\\(\\)"
    )
    {
        applySettings (GameSettings::Min_Search_Depth - 1, GameSettings::Default_Thinking_Time);
    }

    FATAL_CASE(
        "game-settings-search-depth-above-range",
        "Precondition failed at .*game_settings\\.hpp:[0-9]+: isInRange\\(\\)"
    )
    {
        applySettings (GameSettings::Max_Search_Depth + 1, GameSettings::Default_Thinking_Time);
    }

    FATAL_CASE(
        "game-settings-thinking-time-below-range",
        "Precondition failed at .*game_settings\\.hpp:[0-9]+: isInRange\\(\\)"
    )
    {
        applySettings (GameSettings::Default_Search_Depth, GameSettings::Min_Thinking_Time - 1);
    }

    FATAL_CASE(
        "game-settings-thinking-time-above-range",
        "Precondition failed at .*game_settings\\.hpp:[0-9]+: isInRange\\(\\)"
    )
    {
        applySettings (GameSettings::Default_Search_Depth, GameSettings::Max_Thinking_Time + 1);
    }
}
