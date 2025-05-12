#include "remotekeyboardpluginwrapper.h"

RemoteKeyboardPluginWrapper::RemoteKeyboardPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::RemoteKeyboard, parent)
{}

bool RemoteKeyboardPluginWrapper::remoteState() const
{
    return propertyValue<bool>("remoteState", false);
}

void RemoteKeyboardPluginWrapper::sendKeyPress(
    const QString &key, int specialKey, bool shift, bool ctrl, bool alt, bool sendAck)
{
    invokeMethod("sendKeyPress", key, specialKey, shift, ctrl, alt, sendAck);
}

void RemoteKeyboardPluginWrapper::sendQKeyEvent(int key,
                                                Qt::KeyboardModifiers modifiers,
                                                const QString &text,
                                                bool sendAck)
{
    invokeMethod("sendQKeyEvent", key, modifiers, text, sendAck);
}

void RemoteKeyboardPluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin,
                     SIGNAL(remoteStateChanged(bool)),
                     this,
                     SIGNAL(remoteStateChanged(bool)));
}
