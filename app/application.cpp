#include "application.h"
#include "icons.h"
#include "kdeconnectconfig.h"

#include <QIcon>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_MainWindow(nullptr)
    , m_deviceManager(new DeviceManager(this))
{
    QObject::connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

Application::~Application()
{

}

void Application::init()
{
    Icons::initIcons();
    QIcon::setThemeName(QStringLiteral("breeze"));
    m_deviceManager->init();
}

void Application::showMainWindow()
{
    if (m_MainWindow == nullptr) {
        m_MainWindow = new MainWindow();
        m_MainWindow->show();
    } else {
        m_MainWindow->setWindowState((m_MainWindow->windowState() & ~Qt::WindowMinimized)
                                     | Qt::WindowActive);
        m_MainWindow->raise();
        m_MainWindow->activateWindow();
    }
}

void Application::showDeviceWindow(Device::Ptr device)
{
    auto deviceId = device->id();
    auto it = m_deviceWindows.constFind(deviceId);
    if (it == m_deviceWindows.constEnd()) {
        DeviceWindow *deviceWindow = new DeviceWindow(device);
        deviceWindow->show();
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

DeviceManager *Application::deviceManager() const
{
    return m_deviceManager;
}
