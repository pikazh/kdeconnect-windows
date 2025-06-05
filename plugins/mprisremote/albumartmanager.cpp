#include "albumartmanager.h"
#include "plugin_mprisremote_debug.h"

#include "core/kdeconnectconfig.h"

#include <QDir>

AlbumArtManager::AlbumArtManager(const QString &deviceID, QObject *parent)
    : QObject(parent)
    , m_taskSchedule(new TaskScheduler(this, 10))
    , m_db(new AlbumArtDB(this))
    , m_peerDeviceId(deviceID)
{
    QString dbPath = KdeConnectConfig::instance()->deviceDataDir(deviceID).absolutePath();
    QDir().mkpath(dbPath);
    dbPath = dbPath + QDir::separator() + QStringLiteral("albumart");
    QString connectionName = deviceID + QStringLiteral("/albumart");
    m_db->init(connectionName, dbPath);

    QObject::connect(m_taskSchedule,
                     &TaskScheduler::taskFinished,
                     this,
                     &AlbumArtManager::onAlbumDlTaskFinished);

    QObject::connect(m_taskSchedule,
                     &TaskScheduler::taskFailed,
                     this,
                     &AlbumArtManager::onAlbumDlTaskFailed);

    m_taskSchedule->start();
}

AlbumArtManager::~AlbumArtManager()
{
    m_taskSchedule->stop();
}

void AlbumArtManager::downloadAlbumArt(const QString &albumArtUrl,
                                       const QString &host,
                                       quint16 port,
                                       qint64 dataSize)
{
    auto dlTaskPtr = new PeerBufferDownloadTask();
    dlTaskPtr->setPeerDeviceId(m_peerDeviceId);
    dlTaskPtr->setPeerHostAndPort(host, port);
    dlTaskPtr->setContentSize(dataSize);
    dlTaskPtr->setProperty("albumArtUrl", albumArtUrl);

    PeerBufferDownloadTask::Ptr dlTask(dlTaskPtr);
    m_taskSchedule->addTask(dlTask);
    m_albumArtDLTasks.insert(albumArtUrl, dlTask);
}

void AlbumArtManager::onAlbumDlTaskFinished(Task::Ptr task)
{
    PeerBufferDownloadTask *taskPtr = qobject_cast<PeerBufferDownloadTask *>(task.get());
    if (taskPtr != nullptr) {
        QString albumArtUrl = taskPtr->property("albumArtUrl").toString();
        if (!albumArtUrl.isEmpty()) {
            if (taskPtr->isSuccessful()) {
                QByteArray data = taskPtr->downloadedBuffer();
                Q_ASSERT(taskPtr->contentSize() == data.size());
                m_db->insert(albumArtUrl, data);
            }

            m_albumArtDLTasks.remove(albumArtUrl);

            Q_EMIT albumArtDownloadFinished(albumArtUrl);
        }
    }
}

void AlbumArtManager::onAlbumDlTaskFailed(Task::Ptr task, const QString &reason)
{
    PeerBufferDownloadTask *taskPtr = qobject_cast<PeerBufferDownloadTask *>(task.get());
    if (taskPtr != nullptr) {
        QString albumArtUrl = taskPtr->property("albumArtUrl").toString();
        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "download album art with albumArtUrl:" << albumArtUrl << "failed:" << reason;
    }
}

AlbumArtManager::AlbumArtState AlbumArtManager::getAlbumArt(const QString &albumArtUrl,
                                                            QByteArray &data)
{
    if (m_db->query(albumArtUrl, data)) {
        return AlbumArtState::FoundData;
    }

    auto downloadTask = m_albumArtDLTasks.value(albumArtUrl);
    if (downloadTask) {
        return AlbumArtState::Downloading;
    }

    return AlbumArtState::NoData;
}
