#include "remotemousepadpluginwrapper.h"

#include <QPoint>

RemoteMousePadPluginWrapper::RemoteMousePadPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::RemoteMousePad, parent)
{}

void RemoteMousePadPluginWrapper::moveCursor(const QPoint &p)
{
    invokeMethod("moveCursor", p);
}

void RemoteMousePadPluginWrapper::sendSingleClick()
{
    invokeMethod("sendSingleClick");
}

void RemoteMousePadPluginWrapper::sendMiddleClick()
{
    invokeMethod("sendMiddleClick");
}

void RemoteMousePadPluginWrapper::sendRightClick()
{
    invokeMethod("sendRightClick");
}

void RemoteMousePadPluginWrapper::sendSingleHold()
{
    invokeMethod("sendSingleHold");
}

void RemoteMousePadPluginWrapper::sendSingleRelease()
{
    invokeMethod("sendSingleRelease");
}

void RemoteMousePadPluginWrapper::sendScroll(int dx, int dy)
{
    invokeMethod("sendScroll", dx, dy);
}
