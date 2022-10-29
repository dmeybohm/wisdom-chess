#ifndef WISDOM_CHESS_UI_SETTINGS_HPP
#define WISDOM_CHESS_UI_SETTINGS_HPP

#include <QObject>

class UISettings
{
    Q_GADGET
    Q_PROPERTY(bool flipped MEMBER myFlipped READ flipped)

public:
    friend bool operator ==(const UISettings &a, const UISettings&b);
    friend bool operator !=(const UISettings &a, const UISettings&b);

    [[nodiscard]] bool flipped() const;

private:
    bool myFlipped;
};

#endif // WISDOM_CHESS_UI_SETTINGS_HPP
