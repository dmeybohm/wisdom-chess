#pragma once

#include <QObject>

class UISettings
{
    Q_GADGET
    Q_PROPERTY (bool flipped MEMBER my_flipped READ flipped)

public:
    friend bool operator== (const UISettings& a, const UISettings& b);
    friend bool operator!= (const UISettings& a, const UISettings& b);

    [[nodiscard]] bool flipped() const;

private:
    bool my_flipped;
};

