#pragma once

#include "deviceinterface.h"
#include <QQmlEngine>

class DeviceDBusInterface : public OrgKdeKdeconnectDeviceInterface
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("You're not supposed to instantiate DeviceDBusInterface")
public:
    explicit DeviceDBusInterface(const QString &deviceId, QObject *parent = nullptr);
    QString id() const;
private:
    QString m_devId;
};
