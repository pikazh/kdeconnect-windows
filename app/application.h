#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QMap>
#include <QMenu>
#include <QString>
#include <QSystemTrayIcon>

#include "devicemanager.h"
#include "ui/devicewindow.h"
#include "ui/mainwindow.h"

#define APPLICATION (static_cast<Application *>(QCoreApplication::instance()))

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    virtual ~Application() override;

    void init();
    DeviceManager *deviceManager() const;

public Q_SLOTS:
    void showMainWindow();
    void showDeviceWindow(Device::Ptr device);

protected:
    void createSystemTrayIcon();

protected Q_SLOTS:
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void cleanUp();
    void deviceWindowClosing();
    void mainWindowClosing();

private:
    DeviceManager *m_deviceManager;

    MainWindow *m_MainWindow = nullptr;
    QMap<QString, DeviceWindow *> m_deviceWindows;
    QSystemTrayIcon *m_sysTrayIcon = nullptr;
    QMenu *m_trayIconMenu = nullptr;
};

#endif // APPLICATION_H
