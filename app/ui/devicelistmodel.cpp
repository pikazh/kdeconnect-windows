#include "devicelistmodel.h"

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_columns({Name, Type, State})
    , m_darkGreen(10, 10)
    , m_darkYellow(10, 10)
    , m_lightGray(10, 10)
{
    m_darkGreen.fill(Qt::darkGreen);
    m_darkYellow.fill(Qt::darkYellow);
    m_lightGray.fill(Qt::lightGray);
}

int DeviceListModel::columnCount(const QModelIndex &parent) const
{
    return m_columns.size();
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_deviceList.size();
    }
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int column = index.column();
    if (role == Qt::DisplayRole) {
        switch (m_columns[column]) {
        case Name:
            return m_deviceList.at(index.row())->name();
        case Type:
            return m_deviceList.at(index.row())->typeAsString();
        case State:
            auto device = m_deviceList.at(index.row());
            if (device->isPaired() && device->isReachable()) {
                return tr("Device trusted and connected");
            } else if (device->isReachable()) {
                return tr("Device not trusted");
            } else if (device->isPaired()) {
                return tr("Device disconnected");
            }
        }
    } else if (role == Qt::DecorationRole) {
        if (m_columns[column] == Name) {
            auto device = m_deviceList.at(index.row());
            if (device->isPaired() && device->isReachable()) {
                return m_darkGreen;
            } else if (device->isReachable()) {
                return m_darkYellow;
            } else if (device->isPaired()) {
                return m_lightGray;
            }
        }
    }

    return QVariant();
}

QVariant DeviceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (m_columns[section]) {
        case Name:
            return tr("Name");
        case Type:
            return tr("Type");
        case State:
            return tr("State");
        }
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

void DeviceListModel::setDeviceList(const QList<Device::Ptr> list)
{
    beginResetModel();
    this->m_deviceList = list;
    endResetModel();
}

Device::Ptr DeviceListModel::at(size_t index)
{
    return m_deviceList.at(index);
}

DeviceListProxyModel::DeviceListProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
    , m_columns({DeviceListModel::Name, DeviceListModel::Type, DeviceListModel::State})
{}

bool DeviceListProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (m_columns[left.column()] == DeviceListModel::State) {
        DeviceListModel *sourceDeviceModel = static_cast<DeviceListModel *>(sourceModel());
        auto leftDevice = sourceDeviceModel->at(left.row());
        auto rightDevice = sourceDeviceModel->at(right.row());

        int leftPriority = leftDevice->isPaired() ? Paired : Lowest;
        if (leftDevice->isReachable())
            leftPriority = leftPriority | Reachable;

        int rightPriority = rightDevice->isPaired() ? Paired : Lowest;
        if (rightDevice->isReachable())
            rightPriority = rightPriority | Reachable;

        return leftPriority > rightPriority;
    } else {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}
