#pragma once

#include "BasePageProvider.h"
#include "device.h"

class DevicePairPageProvider : public QObject, public BasePageProvider
{
    Q_OBJECT
public:
    DevicePairPageProvider(Device::Ptr device);
    virtual ~DevicePairPageProvider() = default;

    virtual QList<BasePage *> getPages() override;
    virtual QString dialogTitle() override { return tr("Device Pair"); }

private:
    Device::Ptr m_device;
};
