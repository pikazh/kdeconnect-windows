#pragma once

#include "device.h"
#include "kdeconnectcore_export.h"
#include "plugins/pluginid.h"

#include <QSharedPointer>
#include <QWeakPointer>

class KDECONNECTCORE_EXPORT DeviceExtras : public QObject
{
    Q_OBJECT
public:
    struct BatteryInfo
    {
        int chargePercent = -1;
        bool isCharging = false;
    };

    DeviceExtras(Device::Ptr device, QObject *parent = nullptr);
    virtual ~DeviceExtras() = default;

    bool getBatteryInfo(BatteryInfo *batteryInfo);

protected:
    void loadBatteryPlugin();

Q_SIGNALS:
    void batteryInfoUpdated();

protected Q_SLOTS:
    void devicePluginsChanged();

private:
    QWeakPointer<Device> m_weakDevicePointer;
    QHash<PluginId, KdeConnectPlugin *> m_cachePlugins;
};
