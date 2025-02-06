#ifndef DBUSHELPER_H
#define DBUSHELPER_H

#include <QString>

#include "kdeconnectcore_export.h"

class KDECONNECTCORE_EXPORT DBusHelper
{
public:
    DBusHelper() = default;

    void startDBusDaemon();
    void terminalProcesses();
    void filterNonExportableCharacters(QString &src);

    static QString getDbus_daemon();

private:
    const static QString dbus_daemon;
    const static QString kdeconnect_daemon;
    const static QString kdeconnect_app;
};

#endif // DBUSHELPER_H
