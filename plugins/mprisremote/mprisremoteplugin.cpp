/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "mprisremoteplugin.h"
#include "core/device.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_mprisremote_debug.h"

K_PLUGIN_CLASS_WITH_JSON(MprisRemotePlugin, "kdeconnect_mprisremote.json")

void MprisRemotePlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() != PACKET_TYPE_MPRIS)
        return;

    if (np.has(QStringLiteral("player"))) {
        const QString player = np.get<QString>(QStringLiteral("player"));
        if (!m_players.contains(player)) {
            m_players[player] = new MprisRemotePlayer(player, m_albumArtManager, this);
            bindSignals(m_players[player]);
        }
        m_players[player]->parseNetworkPacket(np);
    }

    if (np.has(QStringLiteral("playerList"))) {
        bool isPlayerListChanged = false;
        QStringList players = np.get<QStringList>(QStringLiteral("playerList"));

        // Remove players not available any more
        for (auto iter = m_players.begin(); iter != m_players.end();) {
            if (!players.contains(iter.key())) {
                iter.value()->deleteLater();
                iter = m_players.erase(iter);
                isPlayerListChanged = true;
            } else {
                ++iter;
            }
        }

        // Add new players
        for (const QString &player : players) {
            if (!m_players.contains(player)) {
                m_players[player] = new MprisRemotePlayer(player, m_albumArtManager, this);
                bindSignals(m_players[player]);
                requestPlayerStatus(player);
                isPlayerListChanged = true;
            }
        }

        if (m_players.empty()) {
            m_currentPlayer = QString();
        } else if (!m_players.contains(m_currentPlayer)) {
            m_currentPlayer = m_players.firstKey();
        }

        if (isPlayerListChanged) {
            Q_EMIT playerListChanged();
        }
    }
}

MprisRemotePlugin::MprisRemotePlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_albumArtManager(new AlbumArtManager(device()->id(), this))
{}

long MprisRemotePlugin::position() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->position() : 0;
}

void MprisRemotePlugin::requestPlayerStatus(const QString &player)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST,
                     {{QStringLiteral("player"), player}, {QStringLiteral("requestNowPlaying"), true}, {QStringLiteral("requestVolume"), true}});
    sendPacket(np);
}

void MprisRemotePlugin::requestPlayerList()
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST, {{QStringLiteral("requestPlayerList"), true}});
    sendPacket(np);
}

void MprisRemotePlugin::requestAlbumArt(const QString &player, const QString &album_art_url)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST, {{QStringLiteral("player"), player}, {QStringLiteral("albumArtUrl"), album_art_url}});
    //qInfo(KDECONNECT_PLUGIN_MPRISREMOTE) << "Requesting album art " << np.serialize();
    sendPacket(np);
}

void MprisRemotePlugin::bindSignals(MprisRemotePlayer *player)
{
    QObject::connect(player, &MprisRemotePlayer::controlsChanged, this, [this, player]() {
        Q_EMIT this->controlsChanged(player->identity());
    });

    QObject::connect(player, &MprisRemotePlayer::playingChanged, this, [this, player]() {
        Q_EMIT this->playingChanged(player->identity());
    });

    QObject::connect(player, &MprisRemotePlayer::positionChanged, this, [this, player]() {
        Q_EMIT this->positionChanged(player->identity());
    });

    QObject::connect(player, &MprisRemotePlayer::volumeChanged, this, [this, player]() {
        Q_EMIT this->volumeChanged(player->identity());
    });

    QObject::connect(player, &MprisRemotePlayer::trackInfoChanged, this, [this, player]() {
        Q_EMIT this->trackInfoChanged(player->identity());
    });
}

void MprisRemotePlugin::sendAction(const QString &action)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST, {{QStringLiteral("player"), m_currentPlayer}, {QStringLiteral("action"), action}});
    sendPacket(np);
}

void MprisRemotePlugin::seek(int offset) const
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST, {{QStringLiteral("player"), m_currentPlayer}, {QStringLiteral("Seek"), offset}});
    sendPacket(np);
}

void MprisRemotePlugin::setVolume(int volume)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST, {{QStringLiteral("player"), m_currentPlayer}, {QStringLiteral("setVolume"), volume}});
    sendPacket(np);
}

void MprisRemotePlugin::setPosition(int position)
{
    NetworkPacket np(PACKET_TYPE_MPRIS_REQUEST, {{QStringLiteral("player"), m_currentPlayer}, {QStringLiteral("SetPosition"), position}});
    sendPacket(np);

    m_players[m_currentPlayer]->setPosition(position);
}

void MprisRemotePlugin::setPlayer(const QString &player)
{
    if (m_currentPlayer != player) {
        m_currentPlayer = player;
        requestPlayerStatus(player);
    }
}

bool MprisRemotePlugin::isPlaying() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->playing() : false;
}

int MprisRemotePlugin::length() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->length() : 0;
}

int MprisRemotePlugin::volume() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->volume() : 0;
}

QString MprisRemotePlugin::player() const
{
    if (m_currentPlayer.isEmpty())
        return QString();
    return m_currentPlayer;
}

QStringList MprisRemotePlugin::playerList() const
{
    return m_players.keys();
}

QString MprisRemotePlugin::title() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->title() : QString();
}

QString MprisRemotePlugin::album() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->album() : QString();
}

QByteArray MprisRemotePlugin::albumArtData() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->albumArtData() : QByteArray();
}

QString MprisRemotePlugin::artist() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->artist() : QString();
}

bool MprisRemotePlugin::canSeek() const
{
    auto player = m_players.value(m_currentPlayer);
    return player ? player->canSeek() : false;
}

#include "mprisremoteplugin.moc"
