#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "gamemodel.h"

using namespace wisdom;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    GameModel gameModel;

    qDebug() << "Returned after creating game model";
    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/WisdomChessQtQml/main.qml"_qs);

    qDebug() << "Creating URL";

    engine.rootContext()->setContextProperty("_myGameModel", &gameModel);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
