#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/ui/qml/main/chess_engine.hpp"
#include "wisdom-chess/ui/qml/main/game_model.hpp"
#include "wisdom-chess/ui/qml/main/pieces_model.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"

using namespace wisdom;

int main (int argc, char *argv[])
{
    wisdom::setEmergencyLogger (std::make_shared<ChessEngine::ChessEngineLogger>());
    wisdom::installEmergencyTerminateHandler();

#ifdef USE_OPENGL_GRAPHICS_BACKEND
    // Workaround resizing flickering issue. Seems like the default rendering
    // backends for QML have some issues on my hardware still, so revert to
    // OpenGL which should be more stable.
    QQuickWindow::setGraphicsApi (QSGRendererInterface::GraphicsApi::OpenGL);
#endif

    QGuiApplication app {argc, argv };
    QGuiApplication::setApplicationName (QStringLiteral ("Wisdom Chess"));
    QGuiApplication::setApplicationVersion (QStringLiteral (WISDOM_CHESS_VERSION));
    QGuiApplication::setOrganizationName (QStringLiteral ("daveme"));
    QGuiApplication::setOrganizationDomain (QStringLiteral ("daveme.com"));

    GameModel game_model;
    PiecesModel pieces_model;

    wisdom::ui::registerQmlTypes();

    QObject::connect (&game_model, &GameModel::engineMoved, &pieces_model, &PiecesModel::playerMoved);
    QObject::connect (&game_model, &GameModel::humanMoved, &pieces_model, &PiecesModel::playerMoved);
    QObject::connect (&game_model, &GameModel::gameStarted, &pieces_model, &PiecesModel::newGame);

    QQmlApplicationEngine engine;

    auto main_qml_file = QStringLiteral ("qrc:/qt/qml/WisdomChess/") + QStringLiteral (MAIN_QML_FILE);
    const QUrl url { main_qml_file };

    engine.rootContext()->setContextProperty (QStringLiteral ("_myGameModel"), &game_model);
    engine.rootContext()->setContextProperty (QStringLiteral ("_myPiecesModel"), &pieces_model);

    QObject::connect (&engine, &QQmlApplicationEngine::objectCreationFailed,
                      &app, [] { QCoreApplication::exit (-1); }, Qt::QueuedConnection);

    engine.load (url);

    game_model.start();
    qDebug() << "sceneGraph backend: " << QQuickWindow::sceneGraphBackend();
    return app.exec();
}
