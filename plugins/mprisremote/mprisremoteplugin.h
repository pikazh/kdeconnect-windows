/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "core/plugins/kdeconnectplugin.h"
#include "mprisremoteplayer.h"

#define PACKET_TYPE_MPRIS_REQUEST QStringLiteral("kdeconnect.mpris.request")
#define PACKET_TYPE_MPRIS QStringLiteral("kdeconnect.mpris")

class MprisRemotePlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume)
    Q_PROPERTY(int length READ length)
    Q_PROPERTY(bool isPlaying READ isPlaying)
    Q_PROPERTY(int position READ position WRITE setPosition)
    Q_PROPERTY(QStringList playerList READ playerList)
    Q_PROPERTY(QString player READ player WRITE setPlayer)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString artist READ artist)
    Q_PROPERTY(QString album READ album)
    Q_PROPERTY(QString albumArtFilePath READ albumArtFilePath)
    Q_PROPERTY(bool canSeek READ canSeek)

public:
    using KdeConnectPlugin::KdeConnectPlugin;

    long position() const;
    int volume() const;
    int length() const;
    bool isPlaying() const;
    QStringList playerList() const;
    QString player() const;
    QString title() const;
    QString artist() const;
    QString album() const;
    QString albumArtFilePath() const;
    bool canSeek() const;

    void setVolume(int volume);
    void setPosition(int position);
    void setPlayer(const QString &player);

public Q_SLOTS:
    void seek(int offset) const;
    void requestPlayerList();
    void sendAction(const QString &action);

    void requestAlbumArt(const QString &player, const QString &album_art_url);

protected:
    void bindSignals(MprisRemotePlayer *player);

    virtual void receivePacket(const NetworkPacket &np) override;

Q_SIGNALS:
    void controlsChanged(QString player);
    void trackInfoChanged(QString player);
    void positionChanged(QString player);
    void volumeChanged(QString player);
    void playingChanged(QString player);
    void playerListChanged();

private:
    void requestPlayerStatus(const QString &player);

    QString m_currentPlayer;
    QMap<QString, MprisRemotePlayer *> m_players;
};
