#include "findmyphonepluginwrapper.h"

FindMyPhonePluginWrapper::FindMyPhonePluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::FindMyPhone, parent)
{}

void FindMyPhonePluginWrapper::ring()
{
    invokeMethod("ring");
}
