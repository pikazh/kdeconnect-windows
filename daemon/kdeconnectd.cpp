#include "desktopdaemon.h"

#include <QCoreApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    ::MessageBoxW(0,0,0,0);
    QCoreApplication app(argc, argv);
    qSetMessagePattern(QStringLiteral("%{time} %{category}: %{message}"));

    DesktopDaemon desktopDaemon;
    desktopDaemon.init();

    return app.exec();
}
