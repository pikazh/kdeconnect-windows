/**
 * SPDX-FileCopyrightText: 2018 Jun Bo Bi <jambonmcyeah@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "mpriscontrolplugin-win.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_mpriscontrol_debug.h"

#include <chrono>
#include <random>

#include <QThread>
#include <windows.h>

K_PLUGIN_CLASS_WITH_JSON(MprisControlPlugin, "kdeconnect_mpriscontrol.json")

MprisControlPlugin::MprisControlPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_mediaPlaybackController(MediaPlayBackController::instance())
{
    QObject::connect(m_mediaPlaybackController,
                     &MediaPlayBackController::playerListUpdated,
                     this,
                     &MprisControlPlugin::sendPlayerList);
    QObject::connect(m_mediaPlaybackController,
                     &MediaPlayBackController::mediaPlaybackSessionInfoUpdated,
                     this,
                     &MprisControlPlugin::sendPlaybackInfo);
}

void MprisControlPlugin::onPluginEnabled()
{
    sendPlayerList();
}

void MprisControlPlugin::sendPlayerList()
{
    auto playerList = m_mediaPlaybackController->playerNameList();

    NetworkPacket np(PACKET_TYPE_MPRIS);
    np.set(QStringLiteral("playerList"), playerList);
    //todo
    np.set(QStringLiteral("supportAlbumArtPayload"), false);
    sendPacket(np);
}

void MprisControlPlugin::sendPlaybackInfo(QString playerName)
{
    auto playbackSessionInfo = m_mediaPlaybackController->mediaPlaybackSessionInfo(playerName);
    if (playbackSessionInfo) {
        NetworkPacket np(PACKET_TYPE_MPRIS);
        np.set(QStringLiteral("player"), playerName);

        np.set(QStringLiteral("isPlaying"), playbackSessionInfo->isPlaying);
        np.set(QStringLiteral("shuffle"), playbackSessionInfo->shuffle);
        np.set(QStringLiteral("loopStatus"), playbackSessionInfo->loopStatus);

        np.set(QStringLiteral("canPause"), playbackSessionInfo->canPause);
        np.set(QStringLiteral("canPlay"), playbackSessionInfo->canPlay);
        np.set(QStringLiteral("canGoNext"), playbackSessionInfo->canGoNext);
        np.set(QStringLiteral("canGoPrevious"), playbackSessionInfo->canGoPrevious);
        np.set(QStringLiteral("canSeek"), playbackSessionInfo->canSeek);

        np.set(QStringLiteral("album"), playbackSessionInfo->album);
        np.set(QStringLiteral("artist"), playbackSessionInfo->artist);
        np.set(QStringLiteral("title"), playbackSessionInfo->title);

        np.set(QStringLiteral("pos"), playbackSessionInfo->pos);
        np.set(QStringLiteral("length"), playbackSessionInfo->length);
        // we don't support setting per-app volume levels yet
        np.set(QStringLiteral("volume"), -1);

        sendPacket(np);
    }
}

void MprisControlPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.has(QStringLiteral("playerList"))) {
        return; // Whoever sent this is an mpris client and not an mpris control!
    }

    // Send the player list
    if (np.get<bool>(QStringLiteral("requestPlayerList"))) {
        sendPlayerList();
    }

    const QString player = np.get<QString>(QStringLiteral("player"));
    if (player.isEmpty()) {
        return;
    }

    if (np.get<bool>(QStringLiteral("requestNowPlaying"))) {
        sendPlaybackInfo(player);
    }

    if (np.has(QStringLiteral("action"))) {
        const QString &action = np.get<QString>(QStringLiteral("action"));
        m_mediaPlaybackController->sendCommand(player, action);
    }
    if (np.has(QStringLiteral("setVolume"))) {
        qWarning(KDECONNECT_PLUGIN_MPRISCONTROL) << "Setting volume is not supported";
    }
    if (np.has(QStringLiteral("Seek"))) {
        qint64 offset = np.get<qint64>(QStringLiteral("Seek"));
        m_mediaPlaybackController->sendCommand(player,
                                               QStringLiteral("Seek"),
                                               {{QStringLiteral("offset"), offset}});
    }

    if (np.has(QStringLiteral("SetPosition"))) {
        qint64 position = np.get<qint64>(QStringLiteral("SetPosition"));
        m_mediaPlaybackController->sendCommand(player,
                                               QStringLiteral("SetPosition"),
                                               {{QStringLiteral("position"), position}});
    }

    if (np.has(QStringLiteral("setShuffle"))) {
        m_mediaPlaybackController->sendCommand(player,
                                               QStringLiteral("setShuffle"),
                                               {{QStringLiteral("shuffle"),
                                                 np.get<bool>(QStringLiteral("setShuffle"))}});
    }

    if (np.has(QStringLiteral("setLoopStatus"))) {
        QString loopStatus = np.get<QString>(QStringLiteral("setLoopStatus"));
        m_mediaPlaybackController->sendCommand(player,
                                               QStringLiteral("setLoopStatus"),
                                               {{QStringLiteral("status"), loopStatus}});
    }
}

#include "mpriscontrolplugin-win.moc"
