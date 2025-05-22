#pragma once

#include "pluginwrapperbase.h"

class SftpPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit SftpPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~SftpPluginWrapper() override = default;

public Q_SLOTS:
    void startBrowsing();
};
