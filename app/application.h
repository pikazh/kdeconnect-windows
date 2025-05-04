#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QMap>
#include <QString>

#include "devicemanager.h"
#include "ui/devicewindow.h"
#include "ui/mainwindow.h"

#define APPLICATION (static_cast<Application *>(QCoreApplication::instance()))

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    virtual ~Application();

    void init();
    void showMainWindow();
    void showDeviceWindow(Device::Ptr device);
    DeviceManager *deviceManager() const;

protected Q_SLOTS:
    void cleanUp();
    void deviceWindowClosing();

private:
    DeviceManager *m_deviceManager;

    MainWindow *m_MainWindow;
    QMap<QString, DeviceWindow *> m_deviceWindows;
};

#endif // APPLICATION_H
