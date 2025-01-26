#include "daemondbusinterface.h"
#include "interfaces_debug.h"

DaemonDBusInterface::DaemonDBusInterface(QObject *parent)
    : OrgKdeKdeconnectDaemonInterface(DaemonDBusInterface::activatedService(), QStringLiteral("/modules/kdeconnect"), QDBusConnection::sessionBus(), parent)
{

}

QString DaemonDBusInterface::activatedService()
{
    const QString serviceName = QStringLiteral("org.kde.kdeconnect");

    auto interface = QDBusConnection::sessionBus().interface();
    auto reply = interface->startService(serviceName);
    if (!reply.isValid()) {
        qWarning(KDECONNECT_INTERFACES) << "error activating kdeconnectd:" << reply.error();
    }

    return serviceName;
}
