#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QSessionManager>

#include "desktopdaemon.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DesktopDaemon desktopDaemon;
    desktopDaemon.init();

    return app.exec();
}
