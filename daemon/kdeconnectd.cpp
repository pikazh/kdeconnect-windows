#include <QtWidgets/QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QSessionManager>

#include "desktopdaemon.h"

bool registerDaemonInstance()
{
    const QString daemonServiceName = QStringLiteral("org.kde.kdeconnect.daemon");
    QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
    QDBusConnectionInterface::RegisterServiceReply reply = interface->registerService(daemonServiceName, QDBusConnectionInterface::QueueService);
    return (reply == QDBusConnectionInterface::ServiceRegistered);
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if(!registerDaemonInstance())
    {
        return 0;
    }

    DesktopDaemon desktopDaemon;
    desktopDaemon.init();

    // kdeconnectd is autostarted, so disable session management to speed up startup
    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    return app.exec();
}
