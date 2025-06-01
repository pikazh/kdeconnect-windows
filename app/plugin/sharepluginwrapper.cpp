#include "sharepluginwrapper.h"

SharePluginWrapper::SharePluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::Share, parent)
{}

void SharePluginWrapper::shareText(const QString &text)
{
    invokeMethod("shareText", text);
}

void SharePluginWrapper::shareUrl(const QUrl &url)
{
    invokeMethod("shareUrl", url);
}
