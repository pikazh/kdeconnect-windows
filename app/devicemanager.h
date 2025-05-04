#pragma once

#include <QObject>

#include "backends/linkprovider.h"
#include "device.h"

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    DeviceManager(QObject *parent = nullptr);
    virtual ~DeviceManager();

    void init();
    void unInit();

    Device::Ptr getDevice(const QString &deviceId) const;
    QList<Device::Ptr> devicesList() const;

protected:
    void addDevice(Device::Ptr device);
    void removeDevice(Device::Ptr d);

Q_SIGNALS:
    void deviceAdded(const QString &id);
    void deviceRemoved(const QString &id); // Note that paired devices will never be removed
    void deviceVisibilityChanged(const QString &id, bool isVisible);
    void deviceListChanged(); // Emitted when any of deviceAdded, deviceRemoved or deviceVisibilityChanged is emitted

private Q_SLOTS:
    void onNewDeviceLink(DeviceLink *dl);
    void onDeviceStatusChanged();

private:
    // Different ways to find devices and connect to them
    QSet<LinkProvider *> m_linkProviders;
    // Every known device
    QMap<QString, Device::Ptr> m_devices;
};
