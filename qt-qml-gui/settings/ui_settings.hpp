#ifndef WISDOM_CHESS_UI_SETTINGS_HPP
#define WISDOM_CHESS_UI_SETTINGS_HPP

#include <QObject>

class UISettings
{
    Q_GADGET
    Q_PROPERTY(bool flipped MEMBER myFlipped READ flipped)

public:
    bool operator == (const UISettings&) const = default;

    bool flipped() const;

private:
    bool myFlipped;
};

#endif // WISDOM_CHESS_UI_SETTINGS_HPP
