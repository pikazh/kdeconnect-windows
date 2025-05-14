#include "pingpluginwrapper.h"

PingPluginWrapper::PingPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::Ping, parent)
{}

void PingPluginWrapper::sendPing()
{
    invokeMethod("sendPing");
}
