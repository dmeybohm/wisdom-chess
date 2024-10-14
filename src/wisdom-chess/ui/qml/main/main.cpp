#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include "wisdom-chess/ui/qml/main/game_model.hpp"
#include "wisdom-chess/ui/qml/main/pieces_model.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"

using namespace wisdom;

int main(int argc, char *argv[])
{
#ifdef USE_OPENGL_GRAPHICS_BACKEND
    // Workaround resizing flickering issue. Seems like the default rendering
    // backends for QML have some issues on my hardware still, so revert to
    // OpenGL which should be more stable.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::OpenGL);
#endif

    QGuiApplication app(argc, argv);

    GameModel game_model;
    PiecesModel pieces_model;

    wisdom::ui::registerQmlTypes();

    QObject::connect(&game_model, &GameModel::engineMoved, &pieces_model, &PiecesModel::playerMoved);
    QObject::connect(&game_model, &GameModel::humanMoved, &pieces_model, &PiecesModel::playerMoved);
    QObject::connect(&game_model, &GameModel::gameStarted, &pieces_model, &PiecesModel::newGame);

    QQmlApplicationEngine engine;

    auto main_qml_file = QString { "qrc:/WisdomChess/" } + QString { MAIN_QML_FILE };
    const QUrl url { main_qml_file };

    qDebug() << "Creating URL";

    engine.rootContext()->setContextProperty("_myGameModel", &game_model);
    engine.rootContext()->setContextProperty("_myPiecesModel", &pieces_model);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl & obj_url) {
        if (!obj && url == obj_url)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    game_model.start();
    qDebug() << "sceneGraph backend: " << QQuickWindow::sceneGraphBackend();
    return app.exec();
}
