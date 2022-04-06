#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "gamemodel.h"
#include "piecesmodel.h"
#include "colorenum.h"

using namespace wisdom;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    GameModel gameModel;
    PiecesModel piecesModel;

    ColorEnum::registerQmlTypes();

    QObject::connect(&gameModel, &GameModel::engineMoved, &piecesModel, &PiecesModel::playerMoved);
    QObject::connect(&gameModel, &GameModel::humanMoved, &piecesModel, &PiecesModel::playerMoved);
    QObject::connect(&gameModel, &GameModel::gameStarted, &piecesModel, &PiecesModel::newGame);

    qDebug() << "Returned after creating game model";
    QQmlApplicationEngine engine;

    const QUrl url(u"qrc:/WisdomChessQtQml/main.qml"_qs);

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
    return app.exec();
}
