#include "ui_types.hpp"
#include <QQmlEngine>

void wisdom::ui::registerQmlTypes()
{
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "Color",
        "Not creatable as it is an enum type"
    );
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "Player",
        "Not creatable as it is an enum type"
    );
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "PieceType",
        "Not creatable as it is an enum type"
    );
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "DrawByRepetitionStatus",
        "Not creatable as it is an enum type"
    );
}
