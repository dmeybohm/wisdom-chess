#include "chesscolor.hpp"
#include <QQmlEngine>

void wisdom::chess::registerChessColorQmlType()
{
    qmlRegisterUncreatableMetaObject(
        wisdom::chess::staticMetaObject,
        "wisdom.chess",
        1, 0,
        "ChessColor",
        "Not creatable as it is an enum type"
    );
}
