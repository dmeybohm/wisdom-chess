#include "web_game.hpp"
#include "game_settings.hpp"

int wisdom::WebGame::our_game_id;

auto wisdom::WebGame::new_from_settings (const wisdom::GameSettings& settings) -> wisdom::WebGame*
{
    auto* new_game = new WebGame ( settings.whitePlayer, settings.blackPlayer );

    const auto computer_depth = GameSettings::map_human_depth_to_computer_depth (settings.searchDepth);
    new_game->setMaxDepth (computer_depth);
    new_game->setThinkingTime (std::chrono::seconds { settings.thinkingTime });

    return new_game;
}
