#ifndef WISDOM_CHESS_UI_SETTINGS_HPP
#define WISDOM_CHESS_UI_SETTINGS_HPP

#include <QObject>

#include "ui_types.hpp"

class UISettings
{
    Q_GADGET

public:
    Q_PROPERTY(wisdom::ui::Player whitePlayer
               MEMBER whitePlayer)

    Q_PROPERTY(wisdom::ui::Player blackPlayer
               MEMBER blackPlayer)

    Q_PROPERTY(int maxDepth MEMBER maxDepth)
    Q_PROPERTY(int maxSearchTime MEMBER maxSearchTime)
    Q_PROPERTY(bool flipped MEMBER flipped)

    wisdom::ui::Player whitePlayer = wisdom::ui::Player::Human;
    wisdom::ui::Player blackPlayer = wisdom::ui::Player::Computer;
    int maxDepth = 3;
    int maxSearchTime = 3;
    bool flipped = false;

    bool operator == (const UISettings&) const = default;
};

#endif // WISDOM_CHESS_UI_SETTINGS_HPP
