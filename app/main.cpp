#include "core/dbushelper.h"
#include "icons.h"
#include "notification.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QSystemTrayIcon>
#include <singleapplication.h>
#include <QQmlComponent>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    Icons::initIcons();

    QGuiApplication app(argc, argv);
    SingleApplication single_instance_guard(argc, argv);
    //SingleApplication::instanceStarted()
    //todo: create or show main window when another instance started

    QIcon appIcon = QIcon(QStringLiteral(":/images/kdeconnect.ico"));
    app.setOrganizationDomain("KdeConnect");
    app.setOrganizationName("zyx");
    app.setApplicationName("KdeConnect");
    app.setWindowIcon(appIcon);
    DBusHelper dbusHelper;
    dbusHelper.startDBusDaemon();

    QQmlEngine engine;
    // QQmlApplicationEngine engine;
    // QObject::connect(
    //     &engine,
    //     &QQmlApplicationEngine::objectCreationFailed,
    //     &app,
    //     []() { QCoreApplication::exit(-1); },
    //     Qt::QueuedConnection);

    Notification * notif = new Notification();
    notif->setText(QStringLiteral("dsdsd"));
    notif->notify();

    QQmlComponent mainWindowComponent(&engine, "org.kde.kdeconnect.app", "Main");
    QQmlComponent deviceWindowcomponent(&engine, "org.kde.kdeconnect.app", "DeviceWindow");

    QObject *mainWindow = mainWindowComponent.create();
    QObject *deviceWindow = deviceWindowcomponent.create();
    engine.rootContext()->setContextProperty("deviceWindow", deviceWindow);

    QSystemTrayIcon sysTrayIcon;
    sysTrayIcon.setIcon(appIcon);
    sysTrayIcon.setToolTip(QStringLiteral("KDE Connect"));
    sysTrayIcon.show();
    //engine.loadFromModule("org.kde.kdeconnect.app", "Main");
    //app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
