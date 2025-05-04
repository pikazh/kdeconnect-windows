#include "mprisremotepluginwrapper.h"

MprisRemotePluginWrapper::MprisRemotePluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::MprisRemote, parent)
{}

QString MprisRemotePluginWrapper::album() const
{
    return propertyValue<QString>("album");
}

QString MprisRemotePluginWrapper::albumArtFilePath() const
{
    return propertyValue<QString>("albumArtFilePath");
}

QString MprisRemotePluginWrapper::artist() const
{
    return propertyValue<QString>("artist");
}

bool MprisRemotePluginWrapper::isPlaying() const
{
    return propertyValue<bool>("isPlaying", false);
}

int MprisRemotePluginWrapper::length() const
{
    return propertyValue<int>("length", 0);
}

void MprisRemotePluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin,
                     SIGNAL(controlsChanged(QString)),
                     this,
                     SIGNAL(controlsChanged(QString)));
    QObject::connect(plugin,
                     SIGNAL(trackInfoChanged(QString)),
                     this,
                     SIGNAL(trackInfoChanged(QString)));
    QObject::connect(plugin,
                     SIGNAL(positionChanged(QString)),
                     this,
                     SIGNAL(positionChanged(QString)));
    QObject::connect(plugin, SIGNAL(volumeChanged(QString)), this, SIGNAL(volumeChanged(QString)));
    QObject::connect(plugin, SIGNAL(playingChanged(QString)), this, SIGNAL(playingChanged(QString)));
    QObject::connect(plugin, SIGNAL(playerListChanged()), this, SIGNAL(playerListChanged()));
}
