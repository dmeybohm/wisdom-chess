#ifndef COLORCLASS_H
#define COLORCLASS_H

#include <QObject>

class ColorClass
{
    Q_GADGET

public:
    enum Value {
        White,
        Black,
    };

    Q_ENUM(Value)

    static void registerQmlTypes();

private:
    explicit ColorClass();
};

using ColorEnum = ColorClass::Value;

#endif // COLORCLASS_H
