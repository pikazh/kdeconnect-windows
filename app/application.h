#pragma once

#include "devicemanager.h"

#include <QApplication>
#include <QMap>
#include <QMenu>
#include <QString>
#include <QSystemTrayIcon>

class MainWindow;
class SmsWindow;
class AppSettingsDialog;
class DeviceWindow;

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
    void showSmsConversationsWindow();
    void showAppSettingsDialog();

protected:
    void createSystemTrayIcon();

    void initStyle();
    void initLanguage();

protected Q_SLOTS:
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void cleanUp();
    void deviceWindowClosing();
    void mainWindowClosing();
    void smsWindowClosing();
    void appSettingsDialogClosing();

private:
    DeviceManager *m_deviceManager;

    MainWindow *m_MainWindow = nullptr;
    SmsWindow *m_smsConversationWindow = nullptr;
    AppSettingsDialog *m_appSetingsDlg = nullptr;
    QMap<QString, DeviceWindow *> m_deviceWindows;
    QSystemTrayIcon *m_sysTrayIcon = nullptr;
    QMenu *m_trayIconMenu = nullptr;
};
