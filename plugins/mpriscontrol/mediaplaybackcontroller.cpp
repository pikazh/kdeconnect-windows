#include "mediaplaybackcontroller.h"
#include "plugin_mpriscontrol_debug.h"

#include <chrono>
#include <windows.h>

namespace {
// title Users select this to control the current media player when we can't detect a specific player name like VLC
const QString DEFAULT_PLAYER = QObject::tr("Current Player");
} // namespace

MediaPlaybackImpl::MediaPlaybackImpl(QObject *parent)
    : QObject(parent)
    , m_sessionManager{nullptr}
{}

void MediaPlaybackImpl::init()
{
    winrt::init_apartment();

    m_sessionManager = winrt::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    m_sessionsChangedRevoker = m_sessionManager
                                   .SessionsChanged(winrt::auto_revoke,
                                                    {this, &MediaPlaybackImpl::onSessionsChanged});

    updateAllSessions();
}

void MediaPlaybackImpl::executeCommand(QString playerId, QString cmd, QVariantHash params)
{
    auto it = m_idToSessions.find(playerId);
    if (it == m_idToSessions.end()) {
        qWarning(KDECONNECT_PLUGIN_MPRISCONTROL) << "can not find session, id is" << playerId;
        return;
    }

    auto session = it.value();

    if (cmd == QStringLiteral("Next")) {
        session.TrySkipNextAsync().get();
    } else if (cmd == QStringLiteral("Previous")) {
        session.TrySkipPreviousAsync().get();
    } else if (cmd == QStringLiteral("Pause")) {
        session.TryPauseAsync().get();
    } else if (cmd == QStringLiteral("PlayPause")) {
        session.TryTogglePlayPauseAsync().get();
    } else if (cmd == QStringLiteral("Stop")) {
        session.TryStopAsync().get();
    } else if (cmd == QStringLiteral("Play")) {
        session.TryPlayAsync().get();
    } else if (cmd == QStringLiteral("Seek")) {
        winrt::TimeSpan offset = std::chrono::microseconds(
            qvariant_cast<qint64>(params[QStringLiteral("offset")]));
        session
            .TryChangePlaybackPositionAsync((getSessionTimelinePosition(session) + offset).count())
            .get();
    } else if (cmd == QStringLiteral("SetPosition")) {
        winrt::TimeSpan position = std::chrono::milliseconds(
            qvariant_cast<qint64>(params[QStringLiteral("position")]));
        session
            .TryChangePlaybackPositionAsync(
                (session.GetTimelineProperties().StartTime() + position).count())
            .get();
    } else if (cmd == QStringLiteral("setShuffle")) {
        session.TryChangeShuffleActiveAsync(qvariant_cast<bool>(params[QStringLiteral("shuffle")]));
    } else if (cmd == QStringLiteral("setLoopStatus")) {
        QString loopStatus = qvariant_cast<QString>(params[QStringLiteral("status")]);
        enum class winrt::Windows::Media::MediaPlaybackAutoRepeatMode loopStatusEnumVal;
        if (loopStatus == QStringLiteral("Track")) {
            loopStatusEnumVal = winrt::Windows::Media::MediaPlaybackAutoRepeatMode::Track;
        } else if (loopStatus == QStringLiteral("Playlist")) {
            loopStatusEnumVal = winrt::Windows::Media::MediaPlaybackAutoRepeatMode::List;
        } else {
            loopStatusEnumVal = winrt::Windows::Media::MediaPlaybackAutoRepeatMode::None;
        }
        session.TryChangeAutoRepeatModeAsync(loopStatusEnumVal);
    }
}

bool MediaPlaybackImpl::getSessionIdInExistingList(
    winrt::GlobalSystemMediaTransportControlsSession session, QString &id)
{
    for (auto it = m_idToSessions.begin(); it != m_idToSessions.end(); ++it) {
        if (it.value() == session) {
            id = it.key();
            return true;
        }
    }

    qWarning(KDECONNECT_PLUGIN_MPRISCONTROL)
        << "can not find session :"
        << QString::fromWCharArray(session.SourceAppUserModelId().c_str())
        << "in existed session list";

    return false;
}

void MediaPlaybackImpl::onSessionsChanged(winrt::GlobalSystemMediaTransportControlsSessionManager,
                                          const winrt::SessionsChangedEventArgs &)
{
    updateAllSessions();
}

void MediaPlaybackImpl::onPlaybackInfoChanged(
    winrt::GlobalSystemMediaTransportControlsSession session,
    const winrt::PlaybackInfoChangedEventArgs &)
{
    QString playerId;
    if (!getSessionIdInExistingList(session, playerId)) {
        return;
    }

    updatePlaybackInfoForSession(session, playerId);
}

void MediaPlaybackImpl::onMediaPropertiesChanged(
    winrt::GlobalSystemMediaTransportControlsSession session,
    const winrt::MediaPropertiesChangedEventArgs &)
{
    QString playerId;
    if (!getSessionIdInExistingList(session, playerId)) {
        return;
    }

    updateMediaPropertiesForSession(session, playerId);
}

void MediaPlaybackImpl::onTimelinePropertiesChanged(
    winrt::GlobalSystemMediaTransportControlsSession session,
    const winrt::TimelinePropertiesChangedEventArgs &)
{
    QString playerId;
    if (!getSessionIdInExistingList(session, playerId)) {
        return;
    }

    updateTimelinePropertiesForSession(session, playerId);
}

void MediaPlaybackImpl::updateAllSessions()
{
    auto sessions = m_sessionManager.GetSessions();
    auto sessionsSize = sessions.Size();

    m_playbackInfoChangedHandlers.clear();
    m_mediaPropertiesChangedHandlers.clear();
    m_timelinePropertiesChangedHandlers.clear();

    QHash<QString, winrt::GlobalSystemMediaTransportControlsSession> idToSessions;
    QHash<winrt::GlobalSystemMediaTransportControlsSession, QString> sessionToUniqueNames;
    QHash<QString, winrt::GlobalSystemMediaTransportControlsSession> newlyCreatedSessions;

    for (auto i = 0; i < sessionsSize; ++i) {
        auto session = sessions.GetAt(i);
        winrt::hstring appUserModelId = session.SourceAppUserModelId();
        QString sessionAppUserModelId = QString::fromWCharArray(appUserModelId.c_str());

        idToSessions.insert(sessionAppUserModelId, session);

        winrt::hstring playerName;
        try {
            playerName
                = winrt::AppInfo::GetFromAppUserModelId(appUserModelId).DisplayInfo().DisplayName();
        } catch (winrt::hresult_error e) {
            qWarning(KDECONNECT_PLUGIN_MPRISCONTROL) << QString::fromWCharArray(playerName.c_str())
                                                     << "doesn\'t have a valid AppUserModelID!";
        }

        if (playerName.empty()) {
            playerName = appUserModelId;
        }

        QString uniqueName = QString::fromWCharArray(playerName.c_str());
        int playerCountWithSameName = 2;
        QList<QString> existedUniqueNames = sessionToUniqueNames.values();
        while (existedUniqueNames.contains(uniqueName)) {
            wchar_t buf[1024] = {0};
            wsprintf(buf, L"%s[%d]", playerName.c_str(), playerCountWithSameName++);
            uniqueName = QString::fromWCharArray(buf);
        }

        sessionToUniqueNames.insert(session, uniqueName);

        m_playbackInfoChangedHandlers.push_back(
            session.PlaybackInfoChanged(winrt::auto_revoke,
                                        {this, &MediaPlaybackImpl::onPlaybackInfoChanged}));

        m_mediaPropertiesChangedHandlers.push_back(
            session.MediaPropertiesChanged(winrt::auto_revoke,
                                           {this, &MediaPlaybackImpl::onMediaPropertiesChanged}));

        m_timelinePropertiesChangedHandlers.push_back(
            session.TimelinePropertiesChanged(winrt::auto_revoke,
                                              {this,
                                               &MediaPlaybackImpl::onTimelinePropertiesChanged}));

        auto it = m_idToSessions.find(sessionAppUserModelId);
        if (it == m_idToSessions.end()) {
            // new session created
            newlyCreatedSessions.insert(sessionAppUserModelId, session);
        }
    }

    m_idToSessions = idToSessions;
    m_sessionToUniqueNames = sessionToUniqueNames;

    QHash<QString, QString> playerIdToNames;
    for (auto it = m_idToSessions.begin(); it != m_idToSessions.end(); ++it) {
        playerIdToNames.insert(it.key(), m_sessionToUniqueNames[it.value()]);
    }
    Q_EMIT sessionListUpdated(playerIdToNames);

    for (auto it = newlyCreatedSessions.begin(); it != newlyCreatedSessions.end(); ++it) {
        updatePlaybackInfoForSession(it.value(), it.key());
        updateMediaPropertiesForSession(it.value(), it.key());
        updateTimelinePropertiesForSession(it.value(), it.key());
    }
}

void MediaPlaybackImpl::updateMediaPropertiesForSession(
    winrt::GlobalSystemMediaTransportControlsSession session, const QString &playerId)
{
    QVariantHash vals;
    try {
        auto mediaProperties = session.TryGetMediaPropertiesAsync().get();
        vals.insert(QStringLiteral("title"),
                    QString::fromWCharArray(mediaProperties.Title().c_str()));
        vals.insert(QStringLiteral("artist"),
                    QString::fromWCharArray(mediaProperties.Artist().c_str()));
        vals.insert(QStringLiteral("album"),
                    QString::fromWCharArray(mediaProperties.AlbumTitle().c_str()));

    } catch (winrt::hresult_error e) {
        qWarning(KDECONNECT_PLUGIN_MPRISCONTROL) << "Failed to get media properties";
    }

    try {
        // todo
        auto mediaProperties = session.TryGetMediaPropertiesAsync().get();
        auto thumbnail = mediaProperties.Thumbnail();
        if (thumbnail) {
            auto stream = thumbnail.OpenReadAsync().get();
            if (stream && stream.CanRead()) {
                //IBuffer data = Buffer(stream.Size());
                //data = stream.ReadAsync(data, stream.Size(), InputStreamOptions::None).get();
            }
        }
    } catch (winrt::hresult_error e) {
        qWarning(KDECONNECT_PLUGIN_MPRISCONTROL) << "Failed to get media thumbnail";
    }

    Q_EMIT mediaPropertiesUpdated(playerId, vals);
}

void MediaPlaybackImpl::updateTimelinePropertiesForSession(
    winrt::GlobalSystemMediaTransportControlsSession session, const QString &playerId)
{
    QVariantHash vals;
    auto timelineProperties = session.GetTimelineProperties();
    qint64 pos = std::chrono::duration_cast<std::chrono::milliseconds>(
                     timelineProperties.Position() - timelineProperties.StartTime())
                     .count();
    vals.insert(QStringLiteral("pos"), pos);
    qint64 length = std::chrono::duration_cast<std::chrono::milliseconds>(
                        timelineProperties.EndTime() - timelineProperties.StartTime())
                        .count();
    vals.insert(QStringLiteral("length"), length);

    Q_EMIT timeLinePropertiesUpdated(playerId, vals);
}

winrt::TimeSpan MediaPlaybackImpl::getSessionTimelinePosition(
    winrt::GlobalSystemMediaTransportControlsSession session)
{
    auto playbackInfo = session.GetPlaybackInfo();
    bool isPlaying = playbackInfo.PlaybackStatus()
                     == winrt::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
    auto position = session.GetTimelineProperties().Position();
    if (isPlaying)
        return position + winrt::clock::now() - session.GetTimelineProperties().LastUpdatedTime();
    else
        return position;
}

void MediaPlaybackImpl::updatePlaybackInfoForSession(
    winrt::GlobalSystemMediaTransportControlsSession session, const QString &playerId)
{
    QVariantHash vals;

    auto playbackInfo = session.GetPlaybackInfo();
    auto playbackControls = playbackInfo.Controls();

    const bool isPlaying
        = (playbackInfo.PlaybackStatus()
           == winrt::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing);
    vals.insert(QStringLiteral("isPlaying"), isPlaying);

    if (playbackInfo.IsShuffleActive()) {
        const bool shuffle = playbackInfo.IsShuffleActive().Value();
        vals.insert(QStringLiteral("shuffle"), shuffle);
    }

    QString loopStatus = "None";
    if (playbackInfo.AutoRepeatMode()) {
        switch (playbackInfo.AutoRepeatMode().Value()) {
        case winrt::Windows::Media::MediaPlaybackAutoRepeatMode::List: {
            loopStatus = "Playlist";
            break;
        }
        case winrt::Windows::Media::MediaPlaybackAutoRepeatMode::Track: {
            loopStatus = "Track";
            break;
        }
        default: {
            break;
        }
        }
    }
    vals.insert(QStringLiteral("loopStatus"), loopStatus);

    vals.insert(QStringLiteral("canPause"), playbackControls.IsPauseEnabled());
    vals.insert(QStringLiteral("canPlay"), playbackControls.IsPlayEnabled());
    vals.insert(QStringLiteral("canGoNext"), playbackControls.IsNextEnabled());
    vals.insert(QStringLiteral("canGoPrevious"), playbackControls.IsPreviousEnabled());
    vals.insert(QStringLiteral("canSeek"), playbackControls.IsPlaybackPositionEnabled());

    Q_EMIT playbackInfoUpdated(playerId, vals);
}

MediaPlayBackController *MediaPlayBackController::instance()
{
    static MediaPlayBackController self;
    return &self;
}

MediaPlayBackController::MediaPlayBackController(QObject *parent)
    : QObject(parent)
    , m_mediaPlaybackImplWrapper(new MediaPlaybackImplWrapper(this))
{
    MediaPlaybackImpl *playbackImpl = m_mediaPlaybackImplWrapper->m_mediaPlaybackImpl;
    QObject::connect(playbackImpl,
                     &MediaPlaybackImpl::sessionListUpdated,
                     this,
                     &MediaPlayBackController::onSessionListUpdated);

    QObject::connect(playbackImpl,
                     &MediaPlaybackImpl::playbackInfoUpdated,
                     this,
                     &MediaPlayBackController::onPlaybackInfoUpdated);

    QObject::connect(playbackImpl,
                     &MediaPlaybackImpl::mediaPropertiesUpdated,
                     this,
                     &MediaPlayBackController::onMediaPropertiesUpdated);

    QObject::connect(playbackImpl,
                     &MediaPlaybackImpl::timeLinePropertiesUpdated,
                     this,
                     &MediaPlayBackController::onTimeLinePropertiesUpdated);

    QMetaObject::invokeMethod(playbackImpl, "init", Qt::QueuedConnection);
}

QList<QString> MediaPlayBackController::playerNameList() const
{
    auto list = m_idToPlayerNames.values();
    list.append(DEFAULT_PLAYER);
    return list;
}

QSharedPointer<MediaPlaybackSessionInfo> MediaPlayBackController::mediaPlaybackSessionInfo(
    const QString &playerName)
{
    if (playerName == DEFAULT_PLAYER) {
        QSharedPointer<MediaPlaybackSessionInfo> mediaPlaybackSessionInfoForDefPlayer(
            new MediaPlaybackSessionInfo);
        mediaPlaybackSessionInfoForDefPlayer->canPlay = true;
        mediaPlaybackSessionInfoForDefPlayer->canGoNext = true;
        mediaPlaybackSessionInfoForDefPlayer->canGoPrevious = true;

        return mediaPlaybackSessionInfoForDefPlayer;
    }

    QString playerId;
    if (playerNameToId(playerName, playerId)) {
        auto it = m_playerPlaybackInfos.find(playerId);
        if (it != m_playerPlaybackInfos.end()) {
            return it.value();
        }
    }

    return QSharedPointer<MediaPlaybackSessionInfo>();
}

void MediaPlayBackController::sendCommand(const QString &playerName,
                                          const QString &command,
                                          const QVariantHash &params)
{
    if (playerName == DEFAULT_PLAYER) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;

        if (command == QStringLiteral("PlayPause") || (command == QStringLiteral("Play"))
            || (command == QStringLiteral("Pause"))) {
            input.ki.wVk = VK_MEDIA_PLAY_PAUSE;
            ::SendInput(1, &input, sizeof(INPUT));
        } else if (command == QStringLiteral("Stop")) {
            input.ki.wVk = VK_MEDIA_STOP;
            ::SendInput(1, &input, sizeof(INPUT));
        } else if (command == QStringLiteral("Next")) {
            input.ki.wVk = VK_MEDIA_NEXT_TRACK;
            ::SendInput(1, &input, sizeof(INPUT));
        } else if (command == QStringLiteral("Previous")) {
            input.ki.wVk = VK_MEDIA_PREV_TRACK;
            ::SendInput(1, &input, sizeof(INPUT));
        }

        return;
    }

    QString playerId;
    if (playerNameToId(playerName, playerId)) {
        QMetaObject::invokeMethod(m_mediaPlaybackImplWrapper->m_mediaPlaybackImpl,
                                  "executeCommand",
                                  Qt::QueuedConnection,
                                  playerId,
                                  command,
                                  params);
    }
}

void MediaPlayBackController::onSessionListUpdated(QHash<QString, QString> playerIdtoNames)
{
    m_idToPlayerNames = playerIdtoNames;

    QHash<QString, QSharedPointer<MediaPlaybackSessionInfo>> playbackInfos;
    for (auto it = m_idToPlayerNames.begin(); it != m_idToPlayerNames.end(); ++it) {
        auto it2 = m_playerPlaybackInfos.find(it.key());
        if (it2 == m_playerPlaybackInfos.end()) {
            QSharedPointer<MediaPlaybackSessionInfo> info(new MediaPlaybackSessionInfo);
            playbackInfos.insert(it.key(), info);
        } else {
            playbackInfos.insert(it.key(), it2.value());
        }
    }

    m_playerPlaybackInfos = playbackInfos;

    Q_EMIT playerListUpdated();
}

void MediaPlayBackController::onPlaybackInfoUpdated(QString playerId, QVariantHash infos)
{
    auto it = m_idToPlayerNames.find(playerId);
    if (it != m_idToPlayerNames.end()) {
        auto &sessionInfos = m_playerPlaybackInfos[playerId];

        sessionInfos->isPlaying = qvariant_cast<bool>(infos.value(QStringLiteral("isPlaying")));
        sessionInfos->shuffle = qvariant_cast<bool>(infos.value(QStringLiteral("shuffle")));
        sessionInfos->loopStatus = qvariant_cast<QString>(
            infos.value(QStringLiteral("loopStatus")));

        sessionInfos->canPause = qvariant_cast<bool>(infos.value(QStringLiteral("canPause")));
        sessionInfos->canPlay = qvariant_cast<bool>(infos.value(QStringLiteral("canPlay")));
        sessionInfos->canGoNext = qvariant_cast<bool>(infos.value(QStringLiteral("canGoNext")));
        sessionInfos->canGoPrevious = qvariant_cast<bool>(
            infos.value(QStringLiteral("canGoPrevious")));
        sessionInfos->canSeek = qvariant_cast<bool>(infos.value(QStringLiteral("canSeek")));

        Q_EMIT mediaPlaybackSessionInfoUpdated(it.value());
    }
}

void MediaPlayBackController::onMediaPropertiesUpdated(QString playerId, QVariantHash infos)
{
    auto it = m_idToPlayerNames.find(playerId);
    if (it != m_idToPlayerNames.end()) {
        auto &sessionInfos = m_playerPlaybackInfos[playerId];

        sessionInfos->title = qvariant_cast<QString>(infos.value(QStringLiteral("title")));
        sessionInfos->artist = qvariant_cast<QString>(infos.value(QStringLiteral("artist")));
        sessionInfos->album = qvariant_cast<QString>(infos.value(QStringLiteral("album")));

        Q_EMIT mediaPlaybackSessionInfoUpdated(it.value());
    }
}

void MediaPlayBackController::onTimeLinePropertiesUpdated(QString playerId, QVariantHash infos)
{
    auto it = m_idToPlayerNames.find(playerId);
    if (it != m_idToPlayerNames.end()) {
        auto &sessionInfos = m_playerPlaybackInfos[playerId];

        sessionInfos->pos = qvariant_cast<qint64>(infos.value(QStringLiteral("pos")));
        sessionInfos->length = qvariant_cast<qint64>(infos.value(QStringLiteral("length")));

        Q_EMIT mediaPlaybackSessionInfoUpdated(it.value());
    }
}

bool MediaPlayBackController::playerNameToId(const QString &playerName, QString &id) const
{
    for (auto it = m_idToPlayerNames.begin(); it != m_idToPlayerNames.end(); ++it) {
        if (it.value() == playerName) {
            id = it.key();
            return true;
        }
    }

    return false;
}

MediaPlaybackImplWrapper::MediaPlaybackImplWrapper(QObject *parent)
    : QObject(parent)
{
    m_mediaPlaybackImpl = new MediaPlaybackImpl();
    m_mediaPlaybackImpl->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, m_mediaPlaybackImpl, &QObject::deleteLater);
    m_thread.start();
}

MediaPlaybackImplWrapper::~MediaPlaybackImplWrapper()
{
    m_thread.quit();
    m_thread.wait(1000);
}
