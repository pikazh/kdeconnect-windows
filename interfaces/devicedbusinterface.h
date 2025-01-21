#pragma once
#include "deviceinterface.h"

class DeviceDBusInterface : public OrgKdeKdeconnectDeviceInterface
{
    Q_OBJECT

public:
    explicit DeviceDBusInterface(const QString &deviceId, QObject *parent = nullptr);
    QString id() const;
private:
    QString m_devId;
};
