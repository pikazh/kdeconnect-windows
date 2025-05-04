#pragma once

#include "pluginwrapperbase.h"

class BatteryPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    BatteryPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~BatteryPluginWrapper() override = default;

    Q_PROPERTY(int charge READ charge NOTIFY refreshed)
    int charge() const;

    Q_PROPERTY(bool isCharging READ isCharging NOTIFY refreshed)
    bool isCharging() const;

Q_SIGNALS:
    void refreshed(int charge, bool isCharging);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;
};
