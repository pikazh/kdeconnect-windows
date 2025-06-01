#include "remotecommandspluginwrapper.h"

RemoteCommandsPluginWrapper::RemoteCommandsPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::RemoteCommands, parent)
{}

void RemoteCommandsPluginWrapper::triggerCommand(const QString &key)
{
    invokeMethod("triggerCommand", key);
}

void RemoteCommandsPluginWrapper::editCommands()
{
    invokeMethod("editCommands");
}

QByteArray RemoteCommandsPluginWrapper::commands() const
{
    return propertyValue<QByteArray>("commands");
}

bool RemoteCommandsPluginWrapper::canAddCommand() const
{
    return propertyValue<bool>("canAddCommand");
}

void RemoteCommandsPluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin,
                     SIGNAL(commandsChanged(const QByteArray &)),
                     this,
                     SIGNAL(commandsChanged(const QByteArray &)));
}
