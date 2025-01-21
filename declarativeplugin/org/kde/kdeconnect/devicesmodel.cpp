#include <QIcon>

#include "devicesmodel.h"

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_dbusInterface(new DaemonDBusInterface(this))
{
    connect(this, &QAbstractItemModel::rowsRemoved, this, &DevicesModel::rowsChanged);
    connect(this, &QAbstractItemModel::rowsInserted, this, &DevicesModel::rowsChanged);

    connect(m_dbusInterface, &OrgKdeKdeconnectDaemonInterface::deviceAdded, this, &DevicesModel::onDeviceAdded);
    connect(m_dbusInterface, &OrgKdeKdeconnectDaemonInterface::deviceRemoved, this, &DevicesModel::onDeviceRemoved);
    connect(m_dbusInterface, &OrgKdeKdeconnectDaemonInterface::deviceVisibilityChanged, this, &DevicesModel::onDeviceUpdated);

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(DaemonDBusInterface::activatedService(),
                                                         QDBusConnection::sessionBus(),
                                                         QDBusServiceWatcher::WatchForOwnerChange,
                                                         this);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &DevicesModel::refreshDeviceList);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &DevicesModel::clearDevices);
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row()<0 || index.row() > m_deviceList.size())
    {
        return QVariant();
    }

    Q_ASSERT(m_dbusInterface->isValid());

    DeviceDBusInterface *device = m_deviceList[index.row()];
    Q_ASSERT(device->isValid());

    switch(role)
    {
    case IconModelRole:
    {

        QString icon = data(index, IconNameRole).toString();
        return QIcon::fromTheme(icon);
    }
    case IdModelRole:
        return device->id();
    case NameModelRole:
        return device->name();
    case Qt::ToolTipRole:
    {

        bool trusted = device->isPaired();
        bool reachable = device->isReachable();
        QString status = reachable?(trusted?tr("Device trusted and connected"):tr("Device not trusted")):tr("Device disconnected");
        return status;
    }
    case StatusModelRole:
    {
        int status = StatusFilterFlag::NoFilter;
        if(device->isReachable())
            status |= StatusFilterFlag::Reachable;
        if(device->isPaired())
            status |= StatusFilterFlag::Paired;
        return status;
    }
    case IconNameRole:
        return device->statusIconName();
    case DeviceRole:
        return QVariant::fromValue<QObject*>(device);
    default:
        return QVariant();
    }

}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        // Return size 0 if we are a child because this is not a tree
        return 0;
    }

    return m_deviceList.size();
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    auto names = QAbstractItemModel::roleNames();
    names.insert(NameModelRole, "name");
    names.insert(IdModelRole, "deviceId");
    names.insert(IconNameRole, "iconName");
    names.insert(DeviceRole, "device");
    names.insert(StatusModelRole, "status");
    return names;
}

DeviceDBusInterface *DevicesModel::getDevice(int row) const
{
    if(row < 0 || row >= m_deviceList.size())
    {
        return nullptr;
    }

    return m_deviceList[row];
}

int DevicesModel::rowForDevice(const QString &id) const
{
    for(int i = 0;i< m_deviceList.size();++i)
    {
        if(m_deviceList[i]->id() == id)
        {
            return i;
        }
    }

    return -1;
}

void DevicesModel::onDeviceAdded(const QString &id)
{
    if(rowForDevice(id) >= 0)
    {
        Q_ASSERT_X(false, "onDeviceAdded", "device existed");
        return;
    }

    DeviceDBusInterface *dev = new DeviceDBusInterface(id, this);
    Q_ASSERT(dev->isValid());

    beginInsertRows(QModelIndex(), m_deviceList.size(), m_deviceList.size());
    appendDevice(dev);
    endInsertRows();
}

void DevicesModel::onDeviceRemoved(const QString &id)
{
    int row = rowForDevice(id);
    if(row >=0)
    {
        beginRemoveRows(QModelIndex(), row, row);
        delete m_deviceList.takeAt(row);
        endRemoveRows();
    }
}

void DevicesModel::onDeviceUpdated(const QString &id)
{
    int row = rowForDevice(id);
    Q_ASSERT(row >=0);
    if(row >=0)
    {
        QModelIndex idx = index(row);
        Q_EMIT dataChanged(idx, idx);
    }
}

void DevicesModel::refreshDeviceList()
{
    if(!m_dbusInterface->isValid())
    {
        clearDevices();
        //qWarning(KDECONNECT_INTERFACE) << "dbus interface is not valid";
        return;
    }

    QDBusPendingReply<QStringList> pendingDevicesIds = m_dbusInterface->devices(false, false);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingDevicesIds, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &DevicesModel::receiveDeviceList);
}

void DevicesModel::receiveDeviceList(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    clearDevices();

    QDBusPendingReply<QStringList> pendingDevicesIds = *watcher;
    if(pendingDevicesIds.isError())
    {
        //qWarning(KDECONNECT_INTERFACE) << pendingDevicesIds.error();
    }
    else
    {
        QStringList deviceIds = pendingDevicesIds.value();
        if(!deviceIds.isEmpty())
        {
            beginInsertRows(QModelIndex(), 0, deviceIds.count() - 1);
            for(QString &id : deviceIds)
            {
                appendDevice(new DeviceDBusInterface(id, this));
            }
            endInsertRows();
        }
    }
}

void DevicesModel::clearDevices()
{
    if(!m_deviceList.isEmpty())
    {
        beginRemoveRows(QModelIndex(), 0, m_deviceList.size()-1);
        qDeleteAll(m_deviceList);
        endRemoveRows();
    }
}

void DevicesModel::appendDevice(DeviceDBusInterface *dev)
{
    m_deviceList.append(dev);
    connect(dev, &OrgKdeKdeconnectDeviceInterface::nameChanged,
            this, [this, dev](){
        Q_ASSERT(rowForDevice(dev->id()) >= 0);
        onDeviceUpdated(dev->id());
    });
}
