#pragma once

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QThread>
#include <QVariant>
#include <vector>

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>

namespace winrt {
using namespace Windows::Foundation;
using namespace Windows::Media::Control;
using namespace Windows::Storage::Streams;
using namespace Windows::ApplicationModel;

} // namespace winrt

struct MediaPlaybackSessionInfo
{
    QString title;
    QString album;
    QString artist;
    QString albumArtUrl;

    bool isPlaying = false;
    bool shuffle = false;
    QString loopStatus;

    bool canPause = false;
    bool canPlay = false;
    bool canGoNext = false;
    bool canGoPrevious = false;
    bool canSeek = false;

    qint64 pos = 0;
    qint64 length = 0;
};

class MediaPlaybackImpl : public QObject
{
    Q_OBJECT
public:
    explicit MediaPlaybackImpl(QObject *parent = nullptr);
    virtual ~MediaPlaybackImpl() override = default;

public Q_SLOTS:
    void init();
    void executeCommand(QString playerId, QString cmd, QVariantHash params);

protected:
    bool getSessionIdInExistingList(winrt::GlobalSystemMediaTransportControlsSession session,
                                    QString &name);

    void onSessionsChanged(winrt::GlobalSystemMediaTransportControlsSessionManager,
                           winrt::SessionsChangedEventArgs const &);
    void onPlaybackInfoChanged(winrt::GlobalSystemMediaTransportControlsSession session,
                               winrt::PlaybackInfoChangedEventArgs const &);
    void onMediaPropertiesChanged(winrt::GlobalSystemMediaTransportControlsSession session,
                                  winrt::MediaPropertiesChangedEventArgs const &);
    void onTimelinePropertiesChanged(winrt::GlobalSystemMediaTransportControlsSession session,
                                     winrt::TimelinePropertiesChangedEventArgs const &);

    void updateAllSessions();
    void updatePlaybackInfoForSession(winrt::GlobalSystemMediaTransportControlsSession session,
                                      const QString &playerId);
    void updateMediaPropertiesForSession(winrt::GlobalSystemMediaTransportControlsSession session,
                                         const QString &playerId);
    void updateTimelinePropertiesForSession(winrt::GlobalSystemMediaTransportControlsSession session,
                                            const QString &playerId);

    winrt::TimeSpan getSessionTimelinePosition(
        winrt::GlobalSystemMediaTransportControlsSession session);

Q_SIGNALS:
    void sessionListUpdated(QHash<QString, QString> playerIdtoNames);
    void playbackInfoUpdated(QString playerId, QVariantHash infos);
    void mediaPropertiesUpdated(QString playerId, QVariantHash infos);
    void timeLinePropertiesUpdated(QString playerId, QVariantHash infos);

private:
    winrt::GlobalSystemMediaTransportControlsSessionManager m_sessionManager;
    winrt::GlobalSystemMediaTransportControlsSessionManager::SessionsChanged_revoker
        m_sessionsChangedRevoker;

    std::vector<winrt::GlobalSystemMediaTransportControlsSession::PlaybackInfoChanged_revoker>
        m_playbackInfoChangedHandlers;
    std::vector<winrt::GlobalSystemMediaTransportControlsSession::MediaPropertiesChanged_revoker>
        m_mediaPropertiesChangedHandlers;
    std::vector<winrt::GlobalSystemMediaTransportControlsSession::TimelinePropertiesChanged_revoker>
        m_timelinePropertiesChangedHandlers;

    QHash<QString, winrt::GlobalSystemMediaTransportControlsSession> m_idToSessions;
    QHash<winrt::GlobalSystemMediaTransportControlsSession, QString> m_sessionToUniqueNames;
};

class MediaPlaybackImplWrapper : public QObject
{
    Q_OBJECT
public:
    MediaPlaybackImplWrapper(QObject *parent = nullptr);
    virtual ~MediaPlaybackImplWrapper();

    QThread m_thread;
    MediaPlaybackImpl *m_mediaPlaybackImpl = nullptr;
};

class MediaPlayBackController : public QObject
{
    Q_OBJECT
public:
    static MediaPlayBackController *instance();

    QList<QString> playerNameList() const;
    QSharedPointer<MediaPlaybackSessionInfo> mediaPlaybackSessionInfo(const QString &playerName);

    void sendCommand(const QString &playerName,
                     const QString &command,
                     const QVariantHash &params = QVariantHash());

protected:
    explicit MediaPlayBackController(QObject *parent = nullptr);
    virtual ~MediaPlayBackController() override = default;

protected Q_SLOTS:
    void onSessionListUpdated(QHash<QString, QString> playerIdtoNames);
    void onPlaybackInfoUpdated(QString playerId, QVariantHash infos);
    void onMediaPropertiesUpdated(QString playerId, QVariantHash infos);
    void onTimeLinePropertiesUpdated(QString playerId, QVariantHash infos);

    bool playerNameToId(const QString &playerName, QString &Id) const;

Q_SIGNALS:
    void playerListUpdated();
    void mediaPlaybackSessionInfoUpdated(QString playerName);

private:
    QHash<QString, QSharedPointer<MediaPlaybackSessionInfo>> m_playerPlaybackInfos;
    QHash<QString, QString> m_idToPlayerNames;

    MediaPlaybackImplWrapper *m_mediaPlaybackImplWrapper;
};
