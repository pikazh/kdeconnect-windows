#ifndef DEVICESMODEL_H
#define DEVICESMODEL_H

#include <QAbstractListModel>
#include <QDBusPendingCallWatcher>
#include <QQmlEngine>

#include "daemondbusinterface.h"
#include "devicedbusinterface.h"

class DevicesModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ rowCount NOTIFY rowsChanged)
public:
    enum ModelRoles
    {
        NameModelRole = Qt::DisplayRole,
        IconModelRole = Qt::DecorationRole,
        StatusModelRole = Qt::InitialSortOrderRole,
        IdModelRole = Qt::UserRole,
        IconNameRole,
        DeviceRole
    };
    Q_ENUM(ModelRoles)

    enum StatusFilterFlag
    {
        NoFilter = 0x00,
        Paired = 0x01,
        Reachable = 0x02
    };
    Q_DECLARE_FLAGS(StatusFilterFlags, StatusFilterFlag)
    Q_FLAG(StatusFilterFlags)
    Q_ENUM(StatusFilterFlag)

    explicit DevicesModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

protected:
    DeviceDBusInterface* getDevice(int row) const;
    int rowForDevice(const QString &id) const;

Q_SIGNALS:
    void rowsChanged();

private Q_SLOTS:
    void onDeviceAdded(const QString &id);
    void onDeviceRemoved(const QString &id);
    void onDeviceUpdated(const QString &id);
    void refreshDeviceList();
    void receiveDeviceList(QDBusPendingCallWatcher *watcher);
    void clearDevices();

private:
    void appendDevice(DeviceDBusInterface *dev);

private:
    DaemonDBusInterface* m_dbusInterface;
    QVector<DeviceDBusInterface*> m_deviceList;
};

#endif // DEVICESMODEL_H
