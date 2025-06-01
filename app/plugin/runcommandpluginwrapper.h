#pragma once

#include "pluginwrapperbase.h"

class RunCommandPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit RunCommandPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;

Q_SIGNALS:
    void setup();
};
