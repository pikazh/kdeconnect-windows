#pragma once

#include <QMainWindow>

#include "device.h"
#include "deviceextras.h"
#include "ui/pages/BasePageContainer.h"
#include "ui/widgets/PageContainer.h"

class DeviceWindow : public QMainWindow, public BasePageContainer
{
    Q_OBJECT
public:
    explicit DeviceWindow(Device::Ptr device, QWidget *parent = nullptr);
    virtual ~DeviceWindow() = default;

    virtual bool selectPage(QString pageId) override;
    virtual BasePage *selectedPage() const override;
    virtual BasePage *getPage(QString pageId) override;
    virtual void refreshContainer() override;
    virtual bool requestClose() override;

    Device::Ptr device() { return m_device; }

protected:
    void loadPagesByPairStatus();

    template<typename T>
    void loadPages();

    virtual void closeEvent(QCloseEvent *) override;

protected Q_SLOTS:
    void titleUpdateDeviceBatteryInfo();

Q_SIGNALS:
    void aboutToClose();

private:
    Device::Ptr m_device;
    DeviceExtras *m_deviceExtras = nullptr;
    PageContainer *m_container = nullptr;
    bool m_isPaired = false;
};
