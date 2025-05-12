#include "sftppluginwrapper.h"

SftpPluginWrapper::SftpPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::Sftp, parent)
{}

void SftpPluginWrapper::startBrowsing()
{
    invokeMethod("startBrowsing");
}
