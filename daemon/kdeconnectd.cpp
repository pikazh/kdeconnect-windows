#include "desktopdaemon.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qSetMessagePattern(QStringLiteral("%{time} %{category}: %{message}"));

    DesktopDaemon desktopDaemon;
    desktopDaemon.init();

    return app.exec();
}
