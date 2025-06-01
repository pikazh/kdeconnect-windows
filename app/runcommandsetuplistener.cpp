#include "runcommandsetuplistener.h"
#include "application.h"
#include "core/plugins/pluginid.h"
#include "ui/devicewindow.h"
#include "ui/dialogs/pluginsettingsdialog.h"

RunCommandSetupListener::RunCommandSetupListener(DeviceManager *deviceManager, QObject *parent)
    : QObject{parent}
    , m_deviceManager(deviceManager)
{
    QObject::connect(m_deviceManager,
                     &DeviceManager::deviceAdded,
                     this,
                     &RunCommandSetupListener::registerSetupCallback);

    QObject::connect(m_deviceManager,
                     &DeviceManager::deviceRemoved,
                     this,
                     &RunCommandSetupListener::unRegisterSetupCallback);

    for (auto device : m_deviceManager->devicesList()) {
        registerSetupCallback(device->id());
    }
}

void RunCommandSetupListener::registerSetupCallback(const QString &devId)
{
    auto dev = m_deviceManager->getDevice(devId);
    auto pluginWrapper = new RunCommandPluginWrapper(dev, this);
    QObject::connect(pluginWrapper, &RunCommandPluginWrapper::setup, this, [this, devId]() {
        runCallback(devId);
    });
    m_runCommandPluginWrappers[devId] = pluginWrapper;
}

void RunCommandSetupListener::unRegisterSetupCallback(const QString &devId)
{
    auto it = m_runCommandPluginWrappers.find(devId);
    if (it != m_runCommandPluginWrappers.end()) {
        delete it.value();
        m_runCommandPluginWrappers.erase(it);
    }
}

void RunCommandSetupListener::runCallback(const QString &devId)
{
    auto dev = m_deviceManager->getDevice(devId);
    Q_ASSERT(dev);
    DeviceWindow *deviceWnd = APPLICATION->showDeviceWindow(dev);
    PluginSettingsDialog *pluginSettingDlg = deviceWnd->showPluginSettingsWindow();
    pluginSettingDlg->showPluginConfigDialog(PluginId::RunCommand);
}
