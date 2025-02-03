#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

#include "core/dbushelper.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationDomain("KdeConnect");
    app.setOrganizationName("zyx");
    app.setApplicationName("KdeConnect");

    DBusHelper dbusHelper;
    dbusHelper.startDBusDaemon();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, [](QObject *object, const QUrl &url)
                     {
        qDebug() << url << "created";

    });
    QTimer timer;
    timer.setInterval(3000);
    QObject::connect(&timer, &QTimer::timeout, [&engine](){
        qDebug() << engine.rootObjects();
    });
    timer.start();
    engine.loadFromModule("org.kde.kdeconnect.app", "Main");

    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
