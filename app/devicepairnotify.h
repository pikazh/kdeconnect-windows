#pragma once

#include <QObject>
#include <QPointer>

#include "devicemanager.h"
#include "notification.h"

class DevicePairNotify : public QObject
{
    Q_OBJECT
public:
    explicit DevicePairNotify(DeviceManager *deviceManager, QObject *parent = nullptr);

protected Q_SLOTS:
    void addDevicePairingRequestNotifyCallback(const QString &deviceId);
    void onDevicePairStateChanged(Device::PairState state);
    void onDeviceReachableChanged(bool reachable);

protected:
    void showPairingRequestNotifyForDevice(Device *dev);
    void closePairingRequestNofifyForDevice(Device *dev);

private:
    DeviceManager *m_deviceManager;

    QHash<QString, QPointer<Notification>> m_notifications;
};
