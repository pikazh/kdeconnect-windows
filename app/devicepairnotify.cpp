#include "devicepairnotify.h"
#include "application.h"
#include "core/device.h"

DevicePairNotify::DevicePairNotify(DeviceManager *deviceManager, QObject *parent)
    : QObject{parent}
    , m_deviceManager(deviceManager)
{
    QObject::connect(m_deviceManager,
                     &DeviceManager::deviceAdded,
                     this,
                     &DevicePairNotify::addDevicePairingRequestNotifyCallback);

    for (auto device : m_deviceManager->devicesList()) {
        addDevicePairingRequestNotifyCallback(device->id());
    }
}

void DevicePairNotify::addDevicePairingRequestNotifyCallback(const QString &deviceId)
{
    auto device = m_deviceManager->getDevice(deviceId);
    if (device) {
        QObject::connect(device.get(),
                         &Device::pairStateChanged,
                         this,
                         &DevicePairNotify::onDevicePairStateChanged);
        QObject::connect(device.get(),
                         &Device::reachableChanged,
                         this,
                         &DevicePairNotify::onDeviceReachableChanged);
    }
}

void DevicePairNotify::onDevicePairStateChanged(Device::PairState state)
{
    Device *dev = qobject_cast<Device *>(QObject::sender());
    if (dev != nullptr) {
        if (state == Device::PairState::RequestedByPeer) {
            showPairingRequestNotifyForDevice(dev);
        } else {
            closePairingRequestNofifyForDevice(dev);
        }
    }
}

void DevicePairNotify::onDeviceReachableChanged(bool reachable)
{
    Device *dev = qobject_cast<Device *>(QObject::sender());
    if (dev != nullptr && !reachable) {
        closePairingRequestNofifyForDevice(dev);
    }
}

void DevicePairNotify::showPairingRequestNotifyForDevice(Device *dev)
{
    QString text = QString(tr("Pairing request from %1")).arg(dev->name());
    auto n = Notification::exec(Notification::StandardEvent::Notification,
                                QStringLiteral("KDE Connect"),
                                text);
    auto acceptAction = n->addAction(tr("Accept"));
    auto rejectAction = n->addAction(tr("Reject"));
    auto viewAction = n->addAction(tr("View"));

    QObject::connect(acceptAction, &NotificationAction::activated, dev, &Device::acceptPairing);
    QObject::connect(rejectAction, &NotificationAction::activated, dev, &Device::cancelPairing);
    QObject::connect(viewAction, &NotificationAction::activated, dev, [dev]() {
        APPLICATION->showDeviceWindow(dev->sharedFromThis());
    });

    m_notifications[dev->id()] = n;
}

void DevicePairNotify::closePairingRequestNofifyForDevice(Device *dev)
{
    auto it = m_notifications.find(dev->id());
    if (it != m_notifications.end()) {
        auto p = it.value();
        if (!p.isNull() && !p->isClosed()) {
            p->close();
        }
        m_notifications.erase(it);
    }
}
