#include "game_settings.hpp"

bool operator ==(const GameSettings &a, const GameSettings &b)
{
    return a.myWhitePlayer == b.myWhitePlayer &&
            a.myBlackPlayer == b.myBlackPlayer &&
            a.myMaxDepth == b.myMaxDepth &&
            a.myMaxSearchTime == b.myMaxSearchTime;
}

bool operator !=(const GameSettings &a, const GameSettings &b)
{
    return !operator== (a, b);
}

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
