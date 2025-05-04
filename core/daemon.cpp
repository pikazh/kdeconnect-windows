#include "daemon.h"
#include "core_debug.h"
#include "kdeconnectconfig.h"
#include "backends/lan/lanlinkprovider.h"

Daemon* Daemon::s_instance = nullptr;

class DaemonPrivate
{
public:
    // Different ways to find devices and connect to them
    QSet<LinkProvider *> m_linkProviders;

    // Every known device
    QMap<QString, Device *> m_devices;
};

void Daemon::onNewDeviceLink(DeviceLink *dl)
{
    QString id = dl->deviceId();
    qDebug(KDECONNECT_CORE) << "Device" << id
                            << "established new link.";

    if(d->m_devices.contains(id))
    {
        Device* device = d->m_devices[id];
        bool wasReachable = device->isReachable();
        device->addLink(dl);

        if(!wasReachable)
        {
            Q_EMIT deviceVisibilityChanged(id, true);
            Q_EMIT deviceListChanged();
        }
    }
    else
    {
        qDebug(KDECONNECT_CORE) << "It is a new device: " << dl->deviceInfo().name;
        Device *device = new Device(this, dl->deviceInfo(), false);
        device->addLink(dl);

        addDevice(device);
    }
}

void Daemon::onDeviceStatusChanged()
{
    Device *device = (Device*)sender();
    if(device != nullptr)
    {
        if(!device->isReachable() && !device->isPaired())
        {
            removeDevice(device);
        }
        else
        {
            Q_EMIT deviceVisibilityChanged(device->id(), device->isReachable());
            Q_EMIT deviceListChanged();
        }
    }
}

Daemon::Daemon(QObject *parent)
    : QObject(parent)
    , d(new DaemonPrivate)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
}

Daemon::~Daemon()
{

}

void Daemon::init()
{
    d->m_linkProviders.insert(new LanLinkProvider());

    // Read remembered paired devices
    const QStringList &list = KdeConnectConfig::instance().trustedDevices();
    for (const QString &id : list) {
        DeviceInfo info = KdeConnectConfig::instance().getTrustedDevice(id);
        if (info.isCertificateValid()) {
            Device *d = new Device(this, info, true);
            addDevice(d);
        } else {
            qCDebug(KDECONNECT_CORE)
                << "Certificate for device " << id << "illegal, deleting the device";
            KdeConnectConfig::instance().removeTrustedDevice(id);
        }
    }

    // Listen to new devices
    for (LinkProvider *a : std::as_const(d->m_linkProviders)) {
        connect(a, &LinkProvider::onConnectionReceived, this, &Daemon::onNewDeviceLink);
        a->onStart();
    }
}

QList<Device *> Daemon::devicesList() const
{
    return d->m_devices.values();
}

Device *Daemon::getDevice(const QString &deviceId) const
{
    for(Device* device : std::as_const(d->m_devices))
    {
        if(device->id() == deviceId)
        {
            return device;
        }
    }

    return nullptr;
}

void Daemon::addDevice(Device *device)
{
    QString id = device->id();
    connect(device, &Device::reachableChanged, this, &Daemon::onDeviceStatusChanged);
    connect(device, &Device::pairStateChanged, this, &Daemon::onDeviceStatusChanged);

    // todo: add notification
    d->m_devices[id] = device;

    Q_EMIT deviceAdded(id);
    Q_EMIT deviceListChanged();
}

void Daemon::removeDevice(Device *device)
{
    d->m_devices.remove(device->id());
    Q_EMIT deviceRemoved(device->id());
    Q_EMIT deviceListChanged();

    device->deleteLater();
}

void Daemon::forceOnNetworkChange()
{
    for(LinkProvider* linkProvider : std::as_const(d->m_linkProviders))
    {
        linkProvider->onNetworkChange();
    }
}

QString Daemon::announcedName()
{
    return KdeConnectConfig::instance().name();
}

void Daemon::setAnnouncedName(const QString &name)
{
    QString filteredName = DeviceInfo::filterName(name);
    KdeConnectConfig::instance().setName(filteredName);
    forceOnNetworkChange();
    Q_EMIT announcedNameChanged(filteredName);
}

QStringList Daemon::devices(bool onlyReachable, bool onlyPaired) const
{
    QStringList ret;
    for(Device* dev: std::as_const(d->m_devices))
    {
        if(onlyReachable && !dev->isReachable())
            continue;
        if(onlyPaired && !dev->isPaired())
            continue;

        ret.append(dev->id());
    }

    return ret;
}
