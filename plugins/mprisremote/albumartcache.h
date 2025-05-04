#pragma once

#include "albumartdownloadtask.h"
#include "task/taskscheduler.h"

#include <QCache>
#include <QDir>
#include <QHash>

class AlbumArtCache : public QObject
{
    Q_OBJECT
public:
    class LocalFile
    {
    public:
        QString m_localPath;
        explicit LocalFile(const QString &localPath);
        virtual ~LocalFile();
    };

    struct IndexItem
    {
        enum class Status {
            NOT_FETCHED,
            FETCHING,
            SUCCESS,
            FAILED,
        };
        Status fetchStatus;
        QSharedPointer<LocalFile> file;

        explicit IndexItem(QString localPath)
            : fetchStatus(Status::SUCCESS)
            , file(new LocalFile(localPath))
        {}
        explicit IndexItem(Status fetchStatus)
            : fetchStatus(fetchStatus)
        {
            // Need localPath in this case
            Q_ASSERT(fetchStatus != Status::SUCCESS);
        }

        bool check()
        {
            if (fetchStatus == Status::SUCCESS && file) {
                if (!file->m_localPath.isEmpty() && QFile::exists(file->m_localPath)) {
                    return true;
                }
            }

            return false;
        }
    };

    explicit AlbumArtCache(QObject *parent = nullptr);
    virtual ~AlbumArtCache() override;

    IndexItem indexItem(const QString &albumArtUrl) const;
    void fetchAlbumArt(const QString &deviceID,
                       const QString &albumArtUrl,
                       const QString &host,
                       quint16 port,
                       qint64 dataSize);

protected Q_SLOTS:
    void onAlbumDlTaskFinished(Task::Ptr task);

Q_SIGNALS:
    void albumArtFetchFinished(const QString albumArtUrl);

private:
    TaskScheduler *m_taskSchedule;
    QHash<QString, AlbumArtDownloadTask::Ptr> m_albumArtDLTasks;
    QDir m_cacheDir;
    QCache<QString, IndexItem> m_localCacheIndex;
};
