#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickWindow>

#include "chesscolor.hpp"
#include "gamemodel.hpp"
#include "piecesmodel.hpp"

#include <QSGRendererInterface>

using namespace wisdom;

int main(int argc, char *argv[])
{
    // Workaround resizing flickering issue. Seems like the default rendering
    // backends for QML have some issues on my hardware still, so revert to
    // OpenGL which should be more stable.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::GraphicsApi::OpenGL);

    QGuiApplication app(argc, argv);
    GameModel gameModel;
    PiecesModel piecesModel;

    wisdom::chess::registerChessColorQmlType();

    QObject::connect(&gameModel, &GameModel::engineMoved, &piecesModel, &PiecesModel::playerMoved);
    QObject::connect(&gameModel, &GameModel::humanMoved, &piecesModel, &PiecesModel::playerMoved);
    QObject::connect(&gameModel, &GameModel::gameStarted, &piecesModel, &PiecesModel::newGame);

    qDebug() << "Returned after creating game model";
    QQmlApplicationEngine engine;

    auto mainQmlFile = QString { "qrc:/WisdomChessQtQml/" } + QString { MAIN_QML_FILE };
    const QUrl url { mainQmlFile };

    qDebug() << "Creating URL";

    engine.rootContext()->setContextProperty("_myGameModel", &gameModel);
    engine.rootContext()->setContextProperty("_myPiecesModel", &piecesModel);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    gameModel.start();
    qDebug() << "sceneGraph backend: " << QQuickWindow::sceneGraphBackend();
    return app.exec();
}
