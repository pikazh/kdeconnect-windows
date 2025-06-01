#pragma once

#include "pluginwrapperbase.h"

class RemoteCommandsPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit RemoteCommandsPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~RemoteCommandsPluginWrapper() override = default;

    Q_PROPERTY(QByteArray commands READ commands NOTIFY commandsChanged)
    Q_PROPERTY(bool canAddCommand READ canAddCommand CONSTANT)

    QByteArray commands() const;
    bool canAddCommand() const;

public Q_SLOTS:
    void triggerCommand(const QString &key);
    void editCommands();

Q_SIGNALS:
    void commandsChanged(const QByteArray &commands);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;
};
