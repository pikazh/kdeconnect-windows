#include "deviceextras.h"
#include "plugins/kdeconnectplugin.h"

DeviceExtras::DeviceExtras(Device::Ptr device, QObject *parent)
    : QObject(parent)
    , m_weakDevicePointer(device)
{
    connect(device.get(), SIGNAL(pluginsChanged()), this, SLOT(devicePluginsChanged()));

    loadBatteryPlugin();
}

bool DeviceExtras::getBatteryInfo(BatteryInfo *batteryInfo)
{
    Q_ASSERT(batteryInfo != nullptr);

    auto it = m_cachePlugins.find(PluginId::BatteryMonitor);
    if (it != m_cachePlugins.end()) {
        KdeConnectPlugin *batteryPlugin = it.value();
        batteryInfo->chargePercent = batteryPlugin->property("charge").toInt();
        batteryInfo->isCharging = batteryPlugin->property("isCharging").toBool();
        return true;
    } else {
        return false;
    }
}

void DeviceExtras::loadBatteryPlugin()
{
    auto devicePointer = m_weakDevicePointer.lock();
    if (!devicePointer) {
        return;
    }

    KdeConnectPlugin *batteryPlugin = devicePointer->plugin(
        pluginIdString(PluginId::BatteryMonitor));
    KdeConnectPlugin *oldBatteryPlugin = nullptr;
    auto it = m_cachePlugins.find(PluginId::BatteryMonitor);
    if (it != m_cachePlugins.end())
        oldBatteryPlugin = it.value();

    if (batteryPlugin != nullptr) {
        if (oldBatteryPlugin != batteryPlugin) {
            QObject::connect(batteryPlugin,
                             SIGNAL(refreshed(bool, int)),
                             this,
                             SIGNAL(batteryInfoUpdated()));
            m_cachePlugins[PluginId::BatteryMonitor] = batteryPlugin;
        }

    } else {
        m_cachePlugins.remove(PluginId::BatteryMonitor);
    }

    Q_EMIT batteryInfoUpdated();
}

void DeviceExtras::devicePluginsChanged()
{
    loadBatteryPlugin();
}
