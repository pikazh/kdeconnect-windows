/**
 * SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "mprisremoteplayer.h"
#include "mprisremoteplugin.h"
#include "plugin_mprisremote_debug.h"

#include <QDateTime>
#include <QUuid>

MprisRemotePlayer::MprisRemotePlayer(QString id,
                                     AlbumArtManager *albumArtMng,
                                     MprisRemotePlugin *plugin)
    : QObject(plugin)
    , id(id)
    , m_playing(false)
    , m_canPlay(true)
    , m_canPause(true)
    , m_canGoPrevious(true)
    , m_canGoNext(true)
    , m_volume(50)
    , m_length(0)
    , m_lastPosition(0)
    , m_lastPositionTime()
    , m_title()
    , m_artist()
    , m_album()
    , m_canSeek(false)
    , m_albumArtManager(albumArtMng)
    , m_plugin(plugin)
{
    connect(m_albumArtManager,
            &AlbumArtManager::albumArtDownloadFinished,
            this,
            &MprisRemotePlayer::onAlbumArtDownloadFinished);
}

void MprisRemotePlayer::parseNetworkPacket(const NetworkPacket &np)
{
    if (np.get<bool>(QStringLiteral("transferringAlbumArt"), false)) {
        QString albumArtUrl = np.get<QString>(QStringLiteral("albumArtUrl"), QStringLiteral(""));
        const QVariantMap transferInfo = np.payloadTransferInfo();
        QString host = qvariant_cast<QString>(transferInfo[QStringLiteral("host")]);
        quint16 port = qvariant_cast<quint16>(transferInfo[QStringLiteral("port")]);
        qint64 payLoadSize = np.payloadSize();
        Q_ASSERT(!albumArtUrl.isEmpty() && !host.isEmpty() && port > 0 && payLoadSize > 0);
        if (!albumArtUrl.isEmpty() && !host.isEmpty() && port > 0 && payLoadSize > 0) {
            m_albumArtManager->downloadAlbumArt(albumArtUrl, host, port, payLoadSize);
        }

        return;
    }

    // Track properties
    QString newTitle = np.get<QString>(QStringLiteral("title"), m_title);
    QString newArtist = np.get<QString>(QStringLiteral("artist"), m_artist);
    QString newAlbum = np.get<QString>(QStringLiteral("album"), m_album);
    QString newAlbumArtUrl = np.get<QString>(QStringLiteral("albumArtUrl"), QStringLiteral(""));
    int newLength = np.get<int>(QStringLiteral("length"), m_length);

    bool trackInfoHasChanged = false;
    // Check if they changed
    if (newTitle != m_title || newArtist != m_artist || newAlbum != m_album
        || newLength != m_length) {
        trackInfoHasChanged = true;
        m_title = newTitle;
        m_artist = newArtist;
        m_album = newAlbum;
        m_length = newLength;
    }

    if (newAlbumArtUrl != m_albumArtUrl) {
        trackInfoHasChanged = true;
        // album art changed
        m_albumArtUrl = newAlbumArtUrl;
        m_albumArtData.clear();

        auto state = m_albumArtManager->getAlbumArt(m_albumArtUrl, m_albumArtData);
        if (state == AlbumArtManager::AlbumArtState::NoData) {
            m_plugin->requestAlbumArt(identity(), m_albumArtUrl);
        }
    }

    if (trackInfoHasChanged) {
        Q_EMIT trackInfoChanged();
    }

    // Check volume changes
    int newVolume = np.get<int>(QStringLiteral("volume"), m_volume);
    if (newVolume != m_volume) {
        m_volume = newVolume;
        Q_EMIT volumeChanged();
    }

    if (np.has(QStringLiteral("pos"))) {
        // Check position
        int newLastPosition = np.get<int>(QStringLiteral("pos"), m_lastPosition);
        int positionDiff = qAbs(position() - newLastPosition);
        m_lastPosition = newLastPosition;
        m_lastPositionTime = QDateTime::currentMSecsSinceEpoch();

        // Only consider it seeking if the position changed more than 1 second or the track has changed
        if (qAbs(positionDiff) >= 1000 || trackInfoHasChanged) {
            Q_EMIT positionChanged();
        }
    }

    // Check if we started/stopped playing
    bool newPlaying = np.get<bool>(QStringLiteral("isPlaying"), m_playing);
    if (newPlaying != m_playing) {
        m_playing = newPlaying;
        Q_EMIT playingChanged();
    }

    // Control properties
    bool newCanSeek = np.get<bool>(QStringLiteral("canSeek"), m_canSeek);
    bool newCanPlay = np.get<bool>(QStringLiteral("canPlay"), m_canPlay);
    bool newCanPause = np.get<bool>(QStringLiteral("canPause"), m_canPause);
    bool newCanGoPrevious = np.get<bool>(QStringLiteral("canGoPrevious"), m_canGoPrevious);
    bool newCanGoNext = np.get<bool>(QStringLiteral("canGoNext"), m_canGoNext);
    if (newCanSeek != m_canSeek || newCanPlay != m_canPlay || newCanPause != m_canPause || newCanGoPrevious != m_canGoPrevious || newCanGoNext != m_canGoNext) {
        m_canSeek = newCanSeek;
        m_canPlay = newCanPlay;
        m_canPause = newCanPause;
        m_canGoPrevious = newCanGoPrevious;
        m_canGoNext = newCanGoNext;
        Q_EMIT controlsChanged();
    }
}

long MprisRemotePlayer::position() const
{
    if (m_playing) {
        return m_lastPosition + (QDateTime::currentMSecsSinceEpoch() - m_lastPositionTime);
    } else {
        return m_lastPosition;
    }
}

void MprisRemotePlayer::setPosition(long position)
{
    m_lastPosition = position;
    m_lastPositionTime = QDateTime::currentMSecsSinceEpoch();
}

int MprisRemotePlayer::volume() const
{
    return m_volume;
}

long int MprisRemotePlayer::length() const
{
    return m_length;
}

bool MprisRemotePlayer::playing() const
{
    return m_playing;
}

QString MprisRemotePlayer::title() const
{
    return m_title;
}

QString MprisRemotePlayer::artist() const
{
    return m_artist;
}

QString MprisRemotePlayer::album() const
{
    return m_album;
}

bool MprisRemotePlayer::canSeek() const
{
    return m_canSeek;
}

QString MprisRemotePlayer::identity() const
{
    return id;
}

bool MprisRemotePlayer::canPlay() const
{
    return m_canPlay;
}

bool MprisRemotePlayer::canPause() const
{
    return m_canPause;
}

bool MprisRemotePlayer::canGoPrevious() const
{
    return m_canGoPrevious;
}

bool MprisRemotePlayer::canGoNext() const
{
    return m_canGoNext;
}

QString MprisRemotePlayer::albumArtUrl() const
{
    return m_albumArtUrl;
}

QByteArray MprisRemotePlayer::albumArtData() const
{
    return m_albumArtData;
}

void MprisRemotePlayer::onAlbumArtDownloadFinished(const QString albumArtUrl)
{
    if (m_albumArtUrl == albumArtUrl) {
        auto state = m_albumArtManager->getAlbumArt(albumArtUrl, m_albumArtData);
        if (state == AlbumArtManager::AlbumArtState::FoundData) {
            Q_EMIT trackInfoChanged();
        }
    }
}
