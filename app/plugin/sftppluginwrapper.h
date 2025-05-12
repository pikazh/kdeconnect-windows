#pragma once

#include "pluginwrapperbase.h"

class SftpPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    SftpPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~SftpPluginWrapper() override = default;

public Q_SLOTS:
    void startBrowsing();
};
