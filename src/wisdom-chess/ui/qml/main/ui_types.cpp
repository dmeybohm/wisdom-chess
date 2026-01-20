#include <QQmlEngine>

#include "wisdom-chess/ui/qml/main/ui_types.hpp"

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
    qmlRegisterUncreatableMetaObject(
        wisdom::ui::staticMetaObject,
        "WisdomChess",
        1, 0,
        "Difficulty",
        "Not creatable as it is an enum type"
    );
}
