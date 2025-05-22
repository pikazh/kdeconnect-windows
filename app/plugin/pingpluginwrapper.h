#pragma once

#include "pluginwrapperbase.h"

class PingPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit PingPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~PingPluginWrapper() override = default;

public Q_SLOTS:
    void sendPing();
};
