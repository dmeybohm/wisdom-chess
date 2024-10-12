#include "wisdom-chess/qml/game_settings.hpp"

bool operator== (const GameSettings& a, const GameSettings& b)
{
    return a.my_white_player == b.my_white_player && a.my_black_player == b.my_black_player
        && a.my_max_depth == b.my_max_depth && a.my_max_search_time == b.my_max_search_time;
}

bool operator!= (const GameSettings& a, const GameSettings& b)
{
    return !operator== (a, b);
}

auto GameSettings::whitePlayer() const -> wisdom::ui::Player
{
    return my_white_player;
}

auto GameSettings::blackPlayer() const -> wisdom::ui::Player
{
    return my_black_player;
}

auto GameSettings::maxDepth() const -> int
{
    return my_max_depth;
}

auto GameSettings::maxSearchTime() const -> int
{
    return my_max_search_time;
}
