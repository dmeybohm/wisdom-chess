#include "ui_settings.hpp"

bool operator== (const UISettings& a, const UISettings& b)
{
    return a.my_flipped == b.my_flipped;
}

bool operator!= (const UISettings& a, const UISettings& b)
{
    return !operator== (a, b);
}

bool UISettings::flipped() const
{
    return my_flipped;
}
