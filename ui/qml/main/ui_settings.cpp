#include "wisdom-chess/qml/ui_settings.hpp"

auto 
operator== (const UISettings& a, const UISettings& b)
    -> bool
{
    return a.my_flipped == b.my_flipped;
}

auto 
operator!= (const UISettings& a, const UISettings& b)
    -> bool
{
    return !operator== (a, b);
}

auto 
UISettings::flipped() const
    -> bool
{
    return my_flipped;
}
