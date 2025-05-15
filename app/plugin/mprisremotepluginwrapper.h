#pragma once

#include "pluginwrapperbase.h"

class MprisRemotePluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    MprisRemotePluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~MprisRemotePluginWrapper() override = default;

    Q_PROPERTY(QString album READ album)
    QString album() const;

    Q_PROPERTY(QByteArray albumArtData READ albumArtData)
    QByteArray albumArtData() const;

    Q_PROPERTY(QString artist READ artist)
    QString artist() const;

    Q_PROPERTY(bool canSeek READ canSeek)
    bool canSeek() const;

    Q_PROPERTY(bool isPlaying READ isPlaying)
    bool isPlaying() const;

    Q_PROPERTY(int length READ length)
    int length() const;

    Q_PROPERTY(QString player READ player WRITE setPlayer)
    QString player() const;
    void setPlayer(const QString &value);

    Q_PROPERTY(QStringList playerList READ playerList)
    QStringList playerList() const;

    Q_PROPERTY(int position READ position WRITE setPosition)
    int position() const;
    void setPosition(int value);

    Q_PROPERTY(QString title READ title)
    QString title() const;

    Q_PROPERTY(int volume READ volume WRITE setVolume)
    int volume() const;
    void setVolume(int value);

public Q_SLOTS:
    void requestPlayerList();

    void seek(int offset);

    void sendAction(const QString &action);

Q_SIGNALS:
    void controlsChanged(QString player);
    void trackInfoChanged(QString player);
    void positionChanged(QString player);
    void volumeChanged(QString player);
    void playingChanged(QString player);
    void playerListChanged();

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;
};
