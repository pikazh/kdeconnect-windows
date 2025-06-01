#include "runcommandpluginwrapper.h"

RunCommandPluginWrapper::RunCommandPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::RunCommand, parent)
{}

void RunCommandPluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin, SIGNAL(setup()), this, SIGNAL(setup()));
}
