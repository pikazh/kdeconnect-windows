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

bool MprisRemotePluginWrapper::canSeek() const
{
    return propertyValue<bool>("canSeek", false);
}

bool MprisRemotePluginWrapper::isPlaying() const
{
    return propertyValue<bool>("isPlaying", false);
}

int MprisRemotePluginWrapper::length() const
{
    return propertyValue<int>("length", 0);
}

QString MprisRemotePluginWrapper::player() const
{
    return propertyValue<QString>("player");
}

void MprisRemotePluginWrapper::setPlayer(const QString &value)
{
    setPropertyValue("player", value);
}

QStringList MprisRemotePluginWrapper::playerList() const
{
    return propertyValue<QStringList>("playerList");
}

int MprisRemotePluginWrapper::position() const
{
    return propertyValue<int>("position", 0);
}

void MprisRemotePluginWrapper::setPosition(int value)
{
    setPropertyValue("position", value);
}

QString MprisRemotePluginWrapper::title() const
{
    return propertyValue<QString>("title");
}

int MprisRemotePluginWrapper::volume() const
{
    return propertyValue<int>("volume");
}

void MprisRemotePluginWrapper::setVolume(int value)
{
    setPropertyValue("volume", value);
}

void MprisRemotePluginWrapper::requestPlayerList()
{
    invokeMethod("requestPlayerList");
}

void MprisRemotePluginWrapper::seek(int offset)
{
    invokeMethod("seek", offset);
}

void MprisRemotePluginWrapper::sendAction(const QString &action)
{
    invokeMethod("sendAction", action);
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
