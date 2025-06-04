#pragma once
#include "pluginwrapperbase.h"

class RemoteSystemVolumePluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit RemoteSystemVolumePluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~RemoteSystemVolumePluginWrapper() override = default;

    Q_PROPERTY(QByteArray sinks READ sinks)
    QByteArray sinks();

public Q_SLOTS:
    void sendVolume(const QString &name, int volume);
    void sendMuted(const QString &name, bool muted);

Q_SIGNALS:
    void sinksChanged();
    void volumeChanged(const QString &name, int volume);
    void mutedChanged(const QString &name, bool muted);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;
};
