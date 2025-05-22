#pragma once

#include "pluginwrapperbase.h"

class FindMyPhonePluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit FindMyPhonePluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~FindMyPhonePluginWrapper() override = default;

public Q_SLOTS:
    void ring();
};
