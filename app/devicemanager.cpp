#include "devicemanager.h"
#include "app_debug.h"
#include "backends/lan/lanlinkprovider.h"
#include "kdeconnectconfig.h"

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{}

DeviceManager::~DeviceManager() {}

void DeviceManager::init()
{
    // Read remembered paired devices
    const QStringList &list = KdeConnectConfig::instance()->trustedDevices();
    for (const QString &id : list) {
        DeviceInfo info = KdeConnectConfig::instance()->getTrustedDevice(id);
        if (info.isCertificateValid()) {
            addDevice(makeShared<Device>(this, info, true));
        } else {
            qCDebug(KDECONNECT_APP)
                << "Certificate for device " << id << "illegal, deleting the device";
            KdeConnectConfig::instance()->removeTrustedDevice(id);
        }
    }

    m_linkProviders.insert(new LanLinkProvider(this));
    // Listen to new devices
    for (LinkProvider *a : std::as_const(m_linkProviders)) {
        connect(a, &LinkProvider::onConnectionReceived, this, &DeviceManager::onNewDeviceLink);
        a->onStart();
    }
}

void DeviceManager::unInit()
{
    for (LinkProvider *a : std::as_const(m_linkProviders)) {
        a->onStop();
        a->deleteLater();
    }

    m_linkProviders.clear();
}

Device::Ptr DeviceManager::getDevice(const QString &deviceId) const
{
    for (Device::Ptr device : std::as_const(m_devices)) {
        if (device->id() == deviceId) {
            return device;
        }
    }

    return Device::Ptr();
}

QList<Device::Ptr> DeviceManager::devicesList() const
{
    return m_devices.values();
}

void DeviceManager::refreshNetwokState()
{
    for (LinkProvider *a : std::as_const(m_linkProviders)) {
        qCDebug(KDECONNECT_APP) << "Sending onNetworkChange to:" << a->name();
        a->onNetworkChange();
    }
}

void DeviceManager::addDevice(Device::Ptr device)
{
    QString id = device->id();
    connect(device.get(), &Device::reachableChanged, this, &DeviceManager::onDeviceReachableChanged);
    connect(device.get(), &Device::pairStateChanged, this, &DeviceManager::onDevicePairStateChanged);

    // todo: add notification
    m_devices[id] = device;

    Q_EMIT deviceAdded(id);
    Q_EMIT deviceListChanged();
}

void DeviceManager::removeDevice(Device::Ptr device)
{
    m_devices.remove(device->id());
    Q_EMIT deviceRemoved(device->id());
    Q_EMIT deviceListChanged();
}

void DeviceManager::onNewDeviceLink(DeviceLink *dl)
{
    QString id = dl->deviceId();
    qDebug(KDECONNECT_APP) << "Device" << id << "established new link.";

    if (m_devices.contains(id)) {
        Device::Ptr device = m_devices[id];
        device->addLink(dl);

    } else {
        DeviceInfo deviceInfo = dl->deviceInfo();
        qDebug(KDECONNECT_APP) << "It is a new device: " << deviceInfo.name;
        auto device = makeShared<Device>(this, deviceInfo, false);
        device->addLink(dl);
        addDevice(device);
    }
}

void DeviceManager::onDeviceReachableChanged()
{
    Device *device = (Device *) sender();
    if (device != nullptr) {
        if (!device->isReachable() && !device->isPaired()) {
            removeDevice(device->sharedFromThis());
        } else {
            Q_EMIT deviceVisibilityChanged(device->id(), device->isReachable());
            Q_EMIT deviceListChanged();
        }
    }
}

void DeviceManager::onDevicePairStateChanged()
{
    Device *device = (Device *) sender();
    if (device != nullptr) {
        if (!device->isReachable() && !device->isPaired()) {
            removeDevice(device->sharedFromThis());
        } else {
            Q_EMIT devicePairStateChanged(device->id(), device->pairState());
            Q_EMIT deviceListChanged();
        }
    }
}
