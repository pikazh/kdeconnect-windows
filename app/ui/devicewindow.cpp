#include "devicewindow.h"
#include "plugins/kdeconnectplugin.h"
#include "ui/pages/devicepairpageprovider.h"
#include "ui/pages/devicepluginpagesprovider.h"

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPushButton>

#include <memory>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

DeviceWindow::DeviceWindow(Device::Ptr device, QWidget *parent)
    : QMainWindow(parent)
    , m_device(device)
    , m_batteryPluginWrapper(new BatteryPluginWrapper(device, this))
    , m_sftpPluginWrapper(new SftpPluginWrapper(device, this))
    , m_clipboardPluginWrapper(new ClipboardPluginWrapper(device, this))
    , m_pingPluginWrapper(new PingPluginWrapper(device, this))
    , m_findMyPhonePluginWrapper(new FindMyPhonePluginWrapper(device, this))
    , m_sharePluginWrapper(new SharePluginWrapper(device, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(device->name());
    createToolBar();

    QObject::connect(m_device.get(), &Device::pairStateChanged, this, [this]() {
        if (m_isPaired == m_device->isPaired()) {
            return;
        }

        reloadPages();
        updatePluginSettingsButtonState();
    });

    QObject::connect(m_device.get(),
                     &Device::pluginsChanged,
                     this,
                     &DeviceWindow::updateVisiblePages,
                     Qt::QueuedConnection);

    m_batteryPluginWrapper->init();
    m_sftpPluginWrapper->init();
    m_clipboardPluginWrapper->init();
    m_pingPluginWrapper->init();
    m_findMyPhonePluginWrapper->init();
    m_sharePluginWrapper->init();

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
    QObject::connect(m_pingPluginWrapper,
                     &PingPluginWrapper::pluginLoadedChange,
                     this,
                     &DeviceWindow::updatePingButtonState);
    QObject::connect(m_findMyPhonePluginWrapper,
                     &FindMyPhonePluginWrapper::pluginLoadedChange,
                     this,
                     &DeviceWindow::updateFindMyPhoneButtonState);
    QObject::connect(m_sharePluginWrapper,
                     &SharePluginWrapper::pluginLoadedChange,
                     this,
                     &DeviceWindow::updateSendFilesButtonState);

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
    m_sendClipboardAction->setIcon(RETRIEVE_THEME_ICON("klipper-symbolic"));
    QObject::connect(m_sendClipboardAction,
                     &QAction::triggered,
                     m_clipboardPluginWrapper,
                     &ClipboardPluginWrapper::sendClipboard);

    m_sendFilesAction = new QAction(m_mainToolBar);
    m_sendFilesAction->setText(tr("Send Files"));
    m_sendFilesAction->setIcon(RETRIEVE_THEME_ICON("folder-network"));
    QObject::connect(m_sendFilesAction, &QAction::triggered, this, &DeviceWindow::selectFilesToSend);

    m_pingAction = new QAction(m_mainToolBar);
    m_pingAction->setText(tr("Send Ping"));
    m_pingAction->setIcon(RETRIEVE_THEME_ICON("hands-free"));
    QObject::connect(m_pingAction,
                     &QAction::triggered,
                     m_pingPluginWrapper,
                     &PingPluginWrapper::sendPing);

    m_findMyPhoneAction = new QAction(m_mainToolBar);
    m_findMyPhoneAction->setText(tr("Ring device"));
    m_findMyPhoneAction->setIcon(RETRIEVE_THEME_ICON("irc-voice"));
    QObject::connect(m_findMyPhoneAction,
                     &QAction::triggered,
                     m_findMyPhonePluginWrapper,
                     &FindMyPhonePluginWrapper::ring);

    m_pluginSettingsAction = new QAction(m_mainToolBar);
    m_pluginSettingsAction->setText(tr("Plugin Settings"));
    m_pluginSettingsAction->setIcon(RETRIEVE_THEME_ICON("configure"));
    QObject::connect(m_pluginSettingsAction,
                     &QAction::triggered,
                     this,
                     &DeviceWindow::showPluginSettingsWindow);

    m_mainToolBar->addAction(m_sftpAction);
    m_mainToolBar->addAction(m_sendFilesAction);
    m_mainToolBar->addAction(m_sendClipboardAction);
    m_mainToolBar->addAction(m_pingAction);
    m_mainToolBar->addAction(m_findMyPhoneAction);
    m_mainToolBar->addAction(m_pluginSettingsAction);
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
    updatePingButtonState();
    updateFindMyPhoneButtonState();
    updateSendFilesButtonState();
    updatePluginSettingsButtonState();
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

        battery += tr(")");
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

void DeviceWindow::updatePingButtonState()
{
    m_pingAction->setVisible(m_pingPluginWrapper->isPluginLoaded());
}

void DeviceWindow::updateFindMyPhoneButtonState()
{
    m_findMyPhoneAction->setVisible(m_findMyPhonePluginWrapper->isPluginLoaded());
}

void DeviceWindow::updateSendFilesButtonState()
{
    m_sendFilesAction->setVisible(m_sharePluginWrapper->isPluginLoaded());
}

void DeviceWindow::updatePluginSettingsButtonState()
{
    m_pluginSettingsAction->setVisible(m_device->isReachable() && m_device->isPaired());
}

void DeviceWindow::updateVisiblePages()
{
    refreshContainer();
}

void DeviceWindow::selectFilesToSend()
{
    auto dirSelectDialog = new QFileDialog(this, tr("Select files to send"));
    dirSelectDialog->setFileMode(QFileDialog::ExistingFiles);
    if (dirSelectDialog->exec() == QDialog::Accepted) {
        QStringList selected = dirSelectDialog->selectedFiles();
        m_sharePluginWrapper->shareFiles(selected);
    }
    dirSelectDialog->deleteLater();
}

PluginSettingsDialog *DeviceWindow::showPluginSettingsWindow()
{
    if (m_pluginSettingDlg == nullptr) {
        m_pluginSettingDlg = new PluginSettingsDialog(m_device, this);
        QObject::connect(m_pluginSettingDlg, &PluginSettingsDialog::finished, this, [this]() {
            m_pluginSettingDlg = nullptr;
        });
    }

    m_pluginSettingDlg->showNormal();
    m_pluginSettingDlg->raise();
    m_pluginSettingDlg->activateWindow();
    return m_pluginSettingDlg;
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

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->setContentsMargins(6, 0, 6, 0);
    m_container->addButtons(buttons);
    connect(buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
}
