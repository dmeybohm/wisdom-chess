#pragma once

#include <QtQmlIntegration/qqmlintegration.h>

#include "wisdom-chess/ui/qml/main/game_model.hpp"
#include "wisdom-chess/ui/qml/main/pieces_model.hpp"

class QJSEngine;
class QQmlEngine;

// QML sees the GameModel and PiecesModel that main() created as the
// singletons GameModel and PiecesModel. These wrappers hand the engine
// those instances: were the models to declare the singletons themselves,
// Qt would default-construct its own, since it prefers a constructor to a
// create() function whenever one exists. Set the instances before the
// engine loads.
class GameModelSingleton
{
    Q_GADGET
    QML_FOREIGN (GameModel)
    QML_NAMED_ELEMENT (GameModel)
    QML_SINGLETON

public:
    static void setInstance (GameModel* instance);

    [[nodiscard]] static auto
    create (QQmlEngine* engine, QJSEngine* js_engine)
        -> GameModel*;
};

class PiecesModelSingleton
{
    Q_GADGET
    QML_FOREIGN (PiecesModel)
    QML_NAMED_ELEMENT (PiecesModel)
    QML_SINGLETON

public:
    static void setInstance (PiecesModel* instance);

    [[nodiscard]] static auto
    create (QQmlEngine* engine, QJSEngine* js_engine)
        -> PiecesModel*;
};
