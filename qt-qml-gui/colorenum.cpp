#include "colorenum.h"

#include <QtQml>

void ColorEnum::registerQmlTypes()
{
    qRegisterMetaType<ColorEnumValue>("Color");
    qmlRegisterUncreatableType<ColorEnum>("wisdom.chess", 1, 0, "Color", "Not creatable as it is an enum type");
}

ColorEnum::ColorEnum()
{

}
