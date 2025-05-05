#include "devicewindow.h"
#include "plugins/kdeconnectplugin.h"
#include "ui/pages/devicepairpageprovider.h"
#include "ui/pages/devicepluginpagesprovider.h"

#include <QCloseEvent>
#include <memory>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

DeviceWindow::DeviceWindow(Device::Ptr device, QWidget *parent)
    : QMainWindow(parent)
    , m_device(device)
    , m_batteryPluginWrapper(new BatteryPluginWrapper(device, this))
    , m_sftpPluginWrapper(new SftpPluginWrapper(device, this))
    , m_clipboardPluginWrapper(new ClipboardPluginWrapper(device, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(device->name());
    createToolBar();

    QObject::connect(m_device.get(), &Device::pairStateChanged, this, [this]() {
        if (m_isPaired == m_device->isPaired()) {
            return;
        }

        reloadPages();
    });

    QObject::connect(m_device.get(),
                     &Device::pluginsChanged,
                     this,
                     &DeviceWindow::updateVisiblePages,
                     Qt::QueuedConnection);

    m_batteryPluginWrapper->init();
    m_sftpPluginWrapper->init();
    m_clipboardPluginWrapper->init();
    QObject::connect(m_batteryPluginWrapper,
                     &BatteryPluginWrapper::refreshed,
                     this,
                     &DeviceWindow::titleUpdateDeviceBatteryInfo);
    QObject::connect(m_batteryPluginWrapper,
                     &BatteryPluginWrapper::pluginLoadedChange,
                     this,
                     &DeviceWindow::titleUpdateDeviceBatteryInfo);

    QObject::connect(m_sftpPluginWrapper,
                     &SftpPluginWrapper::pluginLoadedChange,
                     this,
                     &DeviceWindow::updateSftpButtonState);

    QObject::connect(m_clipboardPluginWrapper,
                     &ClipboardPluginWrapper::pluginLoadedChange,
                     this,
                     &DeviceWindow::updateClipBoardButtonState);

    reloadPages();
    updateUI();
}

void DeviceWindow::createToolBar()
{
    m_mainToolBar = new QToolBar(this);
    m_mainToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mainToolBar->setFloatable(false);
    m_mainToolBar->setMovable(false);
    addToolBar(Qt::ToolBarArea::TopToolBarArea, m_mainToolBar);

    m_sftpAction = new QAction(m_mainToolBar);
    m_sftpAction->setText(tr("Browser Device"));
    m_sftpAction->setIcon(RETRIEVE_THEME_ICON("document-open-folder"));
    QObject::connect(m_sftpAction,
                     &QAction::triggered,
                     m_sftpPluginWrapper,
                     &SftpPluginWrapper::startBrowsing);

    m_sendClipboardAction = new QAction(m_mainToolBar);
    m_sendClipboardAction->setText(tr("Send Clipboard"));
    m_sendClipboardAction->setIcon(RETRIEVE_THEME_ICON("klipper"));
    QObject::connect(m_sendClipboardAction,
                     &QAction::triggered,
                     m_clipboardPluginWrapper,
                     &ClipboardPluginWrapper::sendClipboard);

    m_mainToolBar->addAction(m_sftpAction);
    m_mainToolBar->addAction(m_sendClipboardAction);
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

void DeviceWindow::updateUI()
{
    titleUpdateDeviceBatteryInfo();
    updateSftpButtonState();
    updateClipBoardButtonState();
}

void DeviceWindow::reloadPages()
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

    int charge = m_batteryPluginWrapper->charge();
    bool isCharging = m_batteryPluginWrapper->isCharging();
    if (charge >= 0 && charge <= 100) {
        battery = QString(tr(" (Battery: %1%")).arg(charge);

        if (isCharging) {
            battery += tr(", charging");
        }

        battery += QStringLiteral(")");
    }
    setWindowTitle(title + battery);
}

void DeviceWindow::updateSftpButtonState()
{
    m_sftpAction->setVisible(m_sftpPluginWrapper->isPluginLoaded());
}

void DeviceWindow::updateClipBoardButtonState()
{
    m_sendClipboardAction->setVisible(m_clipboardPluginWrapper->isPluginLoaded());
}

void DeviceWindow::updateVisiblePages()
{
    refreshContainer();
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
