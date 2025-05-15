#include "application.h"
#include "devicepairnotify.h"
#include "icons.h"
#include "kdeconnectconfig.h"
#include "notification.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_deviceManager(new DeviceManager(this))
{
    qSetMessagePattern(QStringLiteral(
        "%{time yyyy-MM-ddTHH:mm:ss.zzz} %{if-category}%{category}: %{endif}%{message}"));

    setApplicationName(QStringLiteral("KDEConnect"));
    setApplicationDisplayName(tr("KDEConnect"));

    new DevicePairNotify(m_deviceManager, this);
    QObject::connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

Application::~Application()
{
    if (m_trayIconMenu != nullptr) {
        delete m_trayIconMenu;
        m_trayIconMenu = nullptr;
    }
}

void Application::init()
{
    Icons::initIcons();
    QIcon::setThemeName(QStringLiteral("breeze"));
    setWindowIcon(QIcon(":/kdeconnect.ico"));
    m_deviceManager->init();
    createSystemTrayIcon();
    this->setQuitOnLastWindowClosed(false);
}

void Application::showMainWindow()
{
    if (m_MainWindow == nullptr) {
        m_MainWindow = new MainWindow();
        QObject::connect(m_MainWindow, SIGNAL(aboutToClose()), this, SLOT(mainWindowClosing()));
    }

    m_MainWindow->showNormal();
    m_MainWindow->raise();
    m_MainWindow->activateWindow();
}

void Application::showDeviceWindow(Device::Ptr device)
{
    auto deviceId = device->id();
    auto it = m_deviceWindows.constFind(deviceId);
    if (it == m_deviceWindows.constEnd()) {
        DeviceWindow *deviceWindow = new DeviceWindow(device);
        deviceWindow->showNormal();
        deviceWindow->raise();
        deviceWindow->activateWindow();
        m_deviceWindows.insert(deviceId, deviceWindow);
        QObject::connect(deviceWindow, SIGNAL(aboutToClose()), this, SLOT(deviceWindowClosing()));
    } else {
        auto deviceWindow = it.value();
        deviceWindow->setWindowState((deviceWindow->windowState() & ~Qt::WindowMinimized)
                                     | Qt::WindowActive);
        deviceWindow->raise();
        deviceWindow->activateWindow();
    }
}

void Application::cleanUp()
{
    m_deviceManager->unInit();
}

void Application::deviceWindowClosing()
{
    auto deviceWindow = qobject_cast<DeviceWindow *>(QObject::sender());
    if (deviceWindow != nullptr) {
        auto it = m_deviceWindows.find(deviceWindow->device()->id());
        if (it != m_deviceWindows.end()) {
            m_deviceWindows.erase(it);
        }
    }
}

void Application::mainWindowClosing()
{
    auto mainWindow = qobject_cast<MainWindow *>(QObject::sender());
    Q_ASSERT(mainWindow == m_MainWindow);
    if (mainWindow != nullptr) {
        m_MainWindow = nullptr;
    }
}

DeviceManager *Application::deviceManager() const
{
    return m_deviceManager;
}

void Application::createSystemTrayIcon()
{
    if (m_sysTrayIcon == nullptr) {
        m_sysTrayIcon = new QSystemTrayIcon(this);
        QObject::connect(m_sysTrayIcon,
                         &QSystemTrayIcon::activated,
                         this,
                         &Application::onSystemTrayIconActivated);

        m_trayIconMenu = new QMenu();
        auto showMainWndAction = new QAction(tr("&Show main window"), m_trayIconMenu);
        QObject::connect(showMainWndAction, &QAction::triggered, this, [this]() {
            this->showMainWindow();
        });
        auto quitAction = new QAction(tr("&Quit"), m_trayIconMenu);
        QObject::connect(quitAction,
                         &QAction::triggered,
                         this,
                         &QCoreApplication::quit,
                         Qt::QueuedConnection);

        m_trayIconMenu->addAction(showMainWndAction);
        m_trayIconMenu->addSeparator();
        m_trayIconMenu->addAction(quitAction);
        m_sysTrayIcon->setContextMenu(m_trayIconMenu);
        m_sysTrayIcon->setIcon(windowIcon());
        m_sysTrayIcon->setToolTip(QStringLiteral("KDE Connect"));
        m_sysTrayIcon->show();
    }
}

void Application::onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        showMainWindow();
    }
}
