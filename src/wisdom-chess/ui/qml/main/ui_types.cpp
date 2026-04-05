#include <QQmlEngine>

#include "wisdom-chess/ui/qml/main/ui_types.hpp"

void wisdom::ui::registerQmlTypes()
{
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "Color",
        QStringLiteral ("Not creatable as it is an enum type")
    );
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "Player",
        QStringLiteral ("Not creatable as it is an enum type")
    );
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "PieceType",
        QStringLiteral ("Not creatable as it is an enum type")
    );
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "DrawByRepetitionStatus",
        QStringLiteral ("Not creatable as it is an enum type")
    );
}
