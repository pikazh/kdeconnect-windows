#include "devicewindow.h"
#include "plugins/kdeconnectplugin.h"
#include "ui/pages/devicepairpageprovider.h"
#include "ui/pages/devicepluginpagesprovider.h"

#include <QCloseEvent>
#include <memory>

DeviceWindow::DeviceWindow(Device::Ptr device, QWidget *parent)
    : QMainWindow(parent)
    , m_device(device)
    , m_deviceExtras(new DeviceExtras(device, this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(device->name());

    QObject::connect(m_device.get(), &Device::pairStateChanged, this, [this]() {
        if (m_isPaired == m_device->isPaired()) {
            return;
        }

        loadPagesByPairStatus();
    });

    loadPagesByPairStatus();

    QObject::connect(m_deviceExtras,
                     &DeviceExtras::batteryInfoUpdated,
                     this,
                     &DeviceWindow::titleUpdateDeviceBatteryInfo);

    titleUpdateDeviceBatteryInfo();
}

bool DeviceWindow::selectPage(QString pageId)
{
    return m_container->selectPage(pageId);
}

BasePage *DeviceWindow::selectedPage() const
{
    return m_container->selectedPage();
}

BasePage *DeviceWindow::getPage(QString pageId)
{
    return m_container->getPage(pageId);
}

void DeviceWindow::refreshContainer()
{
    m_container->refreshContainer();
}

bool DeviceWindow::requestClose()
{
    if (m_container->prepareToClose()) {
        close();
        return true;
    }
    return false;
}

void DeviceWindow::loadPagesByPairStatus()
{
    m_isPaired = m_device->isPaired();
    if (!m_isPaired) {
        loadPages<DevicePairPageProvider>();
    } else {
        loadPages<DevicePluginPagesProvider>();
    }
}

void DeviceWindow::closeEvent(QCloseEvent *evt)
{
    emit aboutToClose();
    evt->accept();
}

void DeviceWindow::titleUpdateDeviceBatteryInfo()
{
    QString title = m_device->name();
    QString battery;

    DeviceExtras::BatteryInfo batteryInfo;
    if (m_deviceExtras->getBatteryInfo(&batteryInfo)) {
        if (batteryInfo.chargePercent >= 0 && batteryInfo.chargePercent <= 100) {
            battery = QString(tr(" (Battery: %1%")).arg(batteryInfo.chargePercent);

            if (batteryInfo.isCharging) {
                battery += tr(", charging");
            }

            battery += QStringLiteral(")");
        }
    }

    setWindowTitle(title + battery);
}

template<typename T>
void DeviceWindow::loadPages()
{
    auto provider = std::make_shared<T>(m_device);
    if (m_container != nullptr) {
        m_container->close();
        m_container->deleteLater();
        m_container = nullptr;
    }

    m_container = new PageContainer(provider.get(), "", this);
    m_container->setParentContainer(this);
    setCentralWidget(m_container);
    setContentsMargins(0, 0, 0, 0);
}
