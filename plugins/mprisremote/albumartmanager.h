#pragma once

#include "albumartdb.h"
#include "task/peerbufferdownloadtask.h"
#include "task/taskscheduler.h"

class AlbumArtManager : public QObject
{
    Q_OBJECT
public:
    enum class AlbumArtState {
        FoundData = 0,
        Downloading,
        NoData,
    };

    explicit AlbumArtManager(const QString &deviceID, QObject *parent = nullptr);
    virtual ~AlbumArtManager() override;

    AlbumArtState getAlbumArt(const QString &albumArtUrl, QByteArray &data);
    void downloadAlbumArt(const QString &albumArtUrl,
                          const QString &host,
                          quint16 port,
                          qint64 dataSize);

protected Q_SLOTS:
    void onAlbumDlTaskFinished(Task::Ptr task);
    void onAlbumDlTaskFailed(Task::Ptr task, const QString &reason);

Q_SIGNALS:
    void albumArtDownloadFinished(const QString &albumArtUrl);

private:
    TaskScheduler *m_taskSchedule;
    QHash<QString, PeerBufferDownloadTask::Ptr> m_albumArtDLTasks;
    AlbumArtDB *m_db;
    QString m_peerDeviceId;
};
