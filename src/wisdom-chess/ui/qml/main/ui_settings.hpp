#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class UISettings
{
    Q_GADGET
    QML_VALUE_TYPE (uiSettings)
    Q_PROPERTY (bool flipped 
        MEMBER my_flipped 
        READ flipped)

public:
    friend auto 
    operator== (const UISettings& a, const UISettings& b) 
        -> bool;

    friend auto 
    operator!= (const UISettings& a, const UISettings& b) 
        -> bool;

    [[nodiscard]] auto 
    flipped() const 
        -> bool;

private:
    bool my_flipped = false;
};

