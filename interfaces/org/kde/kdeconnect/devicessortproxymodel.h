#ifndef DEVICESSORTPROXYMODEL_H
#define DEVICESSORTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QQmlEngine>

class DevicesModel;

class DevicesSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit DevicesSortProxyModel(DevicesModel *devicesModel = nullptr);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // DEVICESSORTPROXYMODEL_H
