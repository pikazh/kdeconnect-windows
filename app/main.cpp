#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "core/dbushelper.h"
#include "Windows.h"
int main(int argc, char *argv[])
{
    ::MessageBox(0,0,0,0);
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
    //engine.load(QUrl("qrc:/qml/Main.qml"));
    engine.loadFromModule("org.kde.kdeconnect.app", "Main");
    return app.exec();
}
