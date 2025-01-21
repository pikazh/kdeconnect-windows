#include "devicedbusinterface.h"
#include "daemondbusinterface.h"

DeviceDBusInterface::DeviceDBusInterface(const QString &deviceId, QObject *parent)
    : OrgKdeKdeconnectDeviceInterface(DaemonDBusInterface::activatedService(),
                                      QStringLiteral("/modules/kdeconnect/devices/") + deviceId,
                                      QDBusConnection::sessionBus(),
                                      parent)
    , m_devId(deviceId)
{

}

QString DeviceDBusInterface::id() const
{
    return m_devId;
}
