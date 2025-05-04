#include "albumartcache.h"
#include "plugin_mprisremote_debug.h"

#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

constexpr std::size_t operator""_MB(unsigned long long v)
{
    return 1024u * 1024u * v;
}

static constexpr qsizetype CACHE_SIZE = 5_MB;

AlbumArtCache::AlbumArtCache(QObject *parent)
    : QObject(parent)
    , m_taskSchedule(new TaskScheduler(10, this))
{
    QObject::connect(m_taskSchedule,
                     &TaskScheduler::taskFinished,
                     this,
                     &AlbumArtCache::onAlbumDlTaskFinished);
    m_taskSchedule->start();
    m_localCacheIndex.setMaxCost(100 * 1024);
    m_cacheDir.setPath(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation).append("/albumart"));
    if (!m_cacheDir.exists()) {
        m_cacheDir.mkpath(QStringLiteral("."));
    }
}

AlbumArtCache::~AlbumArtCache()
{
    m_taskSchedule->stop();
}

void AlbumArtCache::fetchAlbumArt(const QString &deviceID,
                                  const QString &albumArtUrl,
                                  const QString &host,
                                  quint16 port,
                                  qint64 dataSize)
{
    auto dlTaskPtr = new AlbumArtDownloadTask();
    dlTaskPtr->setPeerDeviceId(deviceID);
    dlTaskPtr->setPeerHostAndPort(host, port);
    dlTaskPtr->setDownloadDir(m_cacheDir.absolutePath());
    dlTaskPtr->setProperty("albumArtUrl", albumArtUrl);
    dlTaskPtr->setProperty("dataSize", dataSize);

    AlbumArtDownloadTask::Ptr dlTask(dlTaskPtr);
    m_taskSchedule->addTask(dlTask);
    m_albumArtDLTasks.insert(albumArtUrl, dlTask);
}

void AlbumArtCache::onAlbumDlTaskFinished(Task::Ptr task)
{
    AlbumArtDownloadTask *taskPtr = qobject_cast<AlbumArtDownloadTask *>(task.get());
    if (taskPtr != nullptr) {
        QString albumArtUrl = taskPtr->property("albumArtUrl").toString();
        qint64 dataSize = qvariant_cast<qint64>(taskPtr->property("dataSize"));
        if (!albumArtUrl.isEmpty() && dataSize > 0) {
            if (taskPtr->isSuccessful()) {
                QFileInfo localFileInfo(taskPtr->albumArtFilePath());
                Q_ASSERT(localFileInfo.size() == dataSize);
                m_localCacheIndex.insert(albumArtUrl,
                                         new IndexItem(taskPtr->albumArtFilePath()),
                                         dataSize);
            } else {
                m_localCacheIndex.insert(albumArtUrl, new IndexItem(IndexItem::Status::FAILED));
            }

            m_taskSchedule->removeTask(task);
            m_albumArtDLTasks.remove(albumArtUrl);

            Q_EMIT albumArtFetchFinished(albumArtUrl);
        }
    }
}

AlbumArtCache::IndexItem AlbumArtCache::indexItem(const QString &albumArtUrl) const
{
    auto item = m_localCacheIndex.object(albumArtUrl);
    if (item != nullptr && item->check()) {
        return *item;
    }

    auto downloadTask = m_albumArtDLTasks.value(albumArtUrl);
    if (downloadTask) {
        return IndexItem(AlbumArtCache::IndexItem::Status::FETCHING);
    }

    return IndexItem(AlbumArtCache::IndexItem::Status::NOT_FETCHED);
}

AlbumArtCache::LocalFile::LocalFile(const QString &localPath)
    : m_localPath(localPath)
{}

AlbumArtCache::LocalFile::~LocalFile()
{
    if (!QFile::remove(m_localPath)) {
        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "delete album art file:" << m_localPath << "failed.";
    }
}
