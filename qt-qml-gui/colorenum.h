#ifndef COLORENUM_H
#define COLORENUM_H

#include <QObject>

#include "piece.hpp"

class ColorEnum
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
    explicit ColorEnum();
};

using ColorEnumValue = ColorEnum::Value;

constexpr auto mapColor(wisdom::Color color) -> ColorEnumValue
{
    switch (color) {
    case wisdom::Color::White: return ColorEnum::White;
    case wisdom::Color::Black: return ColorEnum::Black;
    default: assert(0); abort();
    }
}

#endif // COLORENUM_H
