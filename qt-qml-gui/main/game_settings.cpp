#include "game_settings.hpp"

auto GameSettings::whitePlayer() const-> wisdom::ui::Player
{
    return myWhitePlayer;
}

auto GameSettings::blackPlayer() const -> wisdom::ui::Player
{
    return myBlackPlayer;
}

auto GameSettings::maxDepth() const -> int
{
    return myMaxDepth;
}

auto GameSettings::maxSearchTime() const -> int
{
    return myMaxSearchTime;
}
