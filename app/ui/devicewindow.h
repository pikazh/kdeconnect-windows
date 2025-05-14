#pragma once

#include <QAction>
#include <QMainWindow>
#include <QToolBar>

#include "device.h"

#include "plugin/batterypluginwrapper.h"
#include "plugin/clipboardpluginwrapper.h"
#include "plugin/findmyphonepluginwrapper.h"
#include "plugin/pingpluginwrapper.h"
#include "plugin/sftppluginwrapper.h"

#include "ui/dialogs/pluginsettingsdialog.h"
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
    void updateUI();
    void createToolBar();

    void reloadPages();
    template<typename T>
    void loadPages();

    virtual void closeEvent(QCloseEvent *) override;

protected Q_SLOTS:
    void titleUpdateDeviceBatteryInfo();
    void updateSftpButtonState();
    void updateClipBoardButtonState();
    void updatePingButtonState();
    void updateFindMyPhoneButtonState();
    void updatePluginSettingsButtonState();

    void updateVisiblePages();

    void showPluginSettingsWindow();

Q_SIGNALS:
    void aboutToClose();

private:
    bool m_isPaired = false;
    Device::Ptr m_device;

    BatteryPluginWrapper *m_batteryPluginWrapper = nullptr;
    SftpPluginWrapper *m_sftpPluginWrapper = nullptr;
    ClipboardPluginWrapper *m_clipboardPluginWrapper = nullptr;
    PingPluginWrapper *m_pingPluginWrapper = nullptr;
    FindMyPhonePluginWrapper *m_findMyPhonePluginWrapper = nullptr;

    PageContainer *m_container = nullptr;

    QToolBar *m_mainToolBar = nullptr;
    QAction *m_sftpAction = nullptr;
    QAction *m_sendClipboardAction = nullptr;
    QAction *m_pingAction = nullptr;
    QAction *m_findMyPhoneAction = nullptr;
    QAction *m_pluginSettingsAction = nullptr;

    PluginSettingsDialog *m_pluginSettingDlg = nullptr;
};
