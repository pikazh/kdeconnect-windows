#pragma once

#include <QAbstractListModel>
#include <QPixmap>
#include <QSortFilterProxyModel>

#include "device.h"

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Column { State = 0, Name };
    explicit DeviceListModel(QObject *parent = nullptr);
    virtual ~DeviceListModel() = default;

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    void setDeviceList(const QList<Device::Ptr> list);
    Device::Ptr at(size_t i);

private:
    QList<Device::Ptr> m_deviceList;
    QList<Column> m_columns;

    QPixmap m_darkGreen;
    QPixmap m_darkYellow;
    QPixmap m_lightGray;
};

class DeviceListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum StatePriority { Lowest = 0, Paired = 1, Reachable = 1 << 1 };
    explicit DeviceListProxyModel(QObject *parent = nullptr);

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QList<DeviceListModel::Column> m_columns;
};
