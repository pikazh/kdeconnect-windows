#include "clipboardpluginwrapper.h"

ClipboardPluginWrapper::ClipboardPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::ClipBoard, parent)
{}

void ClipboardPluginWrapper::sendClipboard()
{
    invokeMethod("sendClipboard");
}
