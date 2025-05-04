#pragma once

#include "BasePageProvider.h"
#include "device.h"

class DevicePluginPagesProvider : public QObject, public BasePageProvider
{
    Q_OBJECT
public:
    DevicePluginPagesProvider(Device::Ptr device);
    virtual ~DevicePluginPagesProvider() = default;

    virtual QList<BasePage *> getPages() override;
    virtual QString dialogTitle() override { return tr("Device Plugins"); }

private:
    Device::Ptr m_device;
};
