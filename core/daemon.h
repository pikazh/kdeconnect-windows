#pragma once

#include <QObject>
#include <QSet>
#include <QList>
#include <QString>

#include "backends/linkprovider.h"
#include "device.h"

class KDECONNECTCORE_EXPORT Daemon : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.daemon")
public:
    explicit Daemon(QObject *parent = nullptr);
    void init();

    QList<Device*> devicesList() const;
    Device* getDevice(const QString &deviceId) const;

    virtual void askPairingConfirmation(Device *device) = 0;
    virtual void reportError(const QString &title, const QString &description) = 0;
    Q_SCRIPTABLE virtual void sendSimpleNotification(const QString &eventId, const QString &title, const QString &text, const QString &iconName) = 0;

protected:
    void addDevice(Device *device);
    void removeDevice(Device *d);

Q_SIGNALS:
    void deviceAdded(const QString &id);
    void deviceRemoved(const QString &id); // Note that paired devices will never be removed
    void deviceVisibilityChanged(const QString &id, bool isVisible);
    void deviceListChanged(); // Emitted when any of deviceAdded, deviceRemoved or deviceVisibilityChanged is emitted
    void announcedNameChanged(const QString &announcedName);
    void customDevicesChanged(const QStringList &customDevices);

public Q_SLOTS:
    Q_SCRIPTABLE void forceOnNetworkChange();
    Q_SCRIPTABLE QString announcedName();
    Q_SCRIPTABLE void setAnnouncedName(const QString &name);
    // Returns a list of ids. The respective devices can be manipulated using the dbus path: "/modules/kdeconnect/Devices/"+id
    Q_SCRIPTABLE QStringList devices(bool onlyReachable = false, bool onlyPaired = false) const;
    virtual void quit() = 0;

private Q_SLOTS:
    void onNewDeviceLink(DeviceLink *dl);
    void onDeviceStatusChanged();

private:
    // Different ways to find devices and connect to them
    QSet<LinkProvider *> m_linkProviders;
    // Every known device
    QMap<QString, Device *> m_devices;

};
