#pragma once

#include "pluginwrapperbase.h"

class RemoteKeyboardPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    RemoteKeyboardPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~RemoteKeyboardPluginWrapper() override = default;

    Q_PROPERTY(bool remoteState READ remoteState NOTIFY remoteStateChanged)
    bool remoteState() const;

public Q_SLOTS:
    void sendKeyPress(const QString &key,
                      int specialKey = 0,
                      bool shift = false,
                      bool ctrl = false,
                      bool alt = false,
                      bool sendAck = false);

    void sendQKeyEvent(int key,
                       Qt::KeyboardModifiers modifiers,
                       const QString &text,
                       bool sendAck = false);

Q_SIGNALS:
    void remoteStateChanged(bool state);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;
};
