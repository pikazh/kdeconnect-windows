#include "batterypluginwrapper.h"

BatteryPluginWrapper::BatteryPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::BatteryMonitor, parent)
{}

int BatteryPluginWrapper::charge() const
{
    return propertyValue<int>("charge", -1);
}

bool BatteryPluginWrapper::isCharging() const
{
    return propertyValue<bool>("isCharging", false);
}

void BatteryPluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin, SIGNAL(refreshed(int, bool)), this, SIGNAL(refreshed(int, bool)));
}
