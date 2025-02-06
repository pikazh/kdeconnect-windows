#include "core/dbushelper.h"
#include "icons.h"
#include "notification.h"
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QTimer>

int main(int argc, char *argv[])
{
    Icons::initIcons();
    QGuiApplication app(argc, argv);
    app.setOrganizationDomain("KdeConnect");
    app.setOrganizationName("zyx");
    app.setApplicationName("KdeConnect");
    app.setWindowIcon(QIcon(QStringLiteral(":/images/kdeconnect.ico")));
    DBusHelper dbusHelper;
    dbusHelper.startDBusDaemon();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    Notification * notif = new Notification();
    notif->setText(QStringLiteral("dsdsd"));
    notif->notify();

    engine.loadFromModule("org.kde.kdeconnect.app", "Main");
    //app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
