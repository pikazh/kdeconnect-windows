#include "remotesystemvolumepluginwrapper.h"

RemoteSystemVolumePluginWrapper::RemoteSystemVolumePluginWrapper(Device::Ptr devicePtr,
                                                                 QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::RemoteSystemVolume, parent)
{}

QByteArray RemoteSystemVolumePluginWrapper::sinks()
{
    return propertyValue<QByteArray>("sinks");
}

void RemoteSystemVolumePluginWrapper::sendVolume(const QString &name, int volume)
{
    invokeMethod("sendVolume", name, volume);
}

void RemoteSystemVolumePluginWrapper::sendMuted(const QString &name, bool muted)
{
    invokeMethod("sendMuted", name, muted);
}

void RemoteSystemVolumePluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin, SIGNAL(sinksChanged()), this, SIGNAL(sinksChanged()));

    QObject::connect(plugin,
                     SIGNAL(volumeChanged(const QString &, int)),
                     this,
                     SIGNAL(volumeChanged(const QString &, int)));

    QObject::connect(plugin,
                     SIGNAL(mutedChanged(const QString &, bool)),
                     this,
                     SIGNAL(mutedChanged(const QString &, bool)));
}
