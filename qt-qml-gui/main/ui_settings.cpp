#include "ui_settings.hpp"


bool operator ==(const UISettings &a, const UISettings &b)
{
    return a.myFlipped == b.myFlipped;
}

bool operator != (const UISettings &a, const UISettings &b)
{
    return !operator== (a, b);
}

bool UISettings::flipped() const
{
    return myFlipped;
}
