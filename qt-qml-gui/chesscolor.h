#ifndef WISDOM_CHESSCOLOR_H
#define WISDOM_CHESSCOLOR_H

#include <QObject>

#include "piece.hpp"

namespace wisdom::chess
{
    Q_NAMESPACE

    enum class ChessColor
    {
        White,
        Black,
    };

    Q_ENUM_NS(ChessColor)

    auto toString(ChessColor color) -> QString;
    void registerChessColorQmlType();

    constexpr auto mapColor(wisdom::Color color) -> ChessColor
    {
        switch (color) {
        case wisdom::Color::White: return ChessColor::White;
        case wisdom::Color::Black: return ChessColor::Black;
        default: assert(0); abort();
        }
    }
};

#endif // WISDOM_CHESSCOLOR_H
