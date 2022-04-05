#include "colorclass.h"

#include <QtQml>

void ColorClass::registerQmlTypes()
{
    qRegisterMetaType<ColorEnum>("Color");
    qmlRegisterUncreatableType<ColorClass>("wisdom.chess", 1, 0, "Color", "Not creatable as it is an enum type");
}

ColorClass::ColorClass()
{

}
