#include "fileshareserver.h"
#include "common.h"
#include "plugin_share_debug.h"

#include "core/backends/lan/lanlinkprovider.h"

#include <QNetworkProxy>

#define MIN_FILE_SHARE_SERVICE_PORT (2000)
#define MAX_FILE_SHARE_SERVICE_PORT (2050)

FileShareServer::FileShareServer(QSharedPointer<TaskScheduler> sendFilesTaskSchedule,
                                 QString deviceId,
                                 QObject *parent)
    : QObject{parent}
    , m_sslServer(new QSslServer(this))
    , m_deviceId(deviceId)
    , m_sendFilesTaskSchedule(sendFilesTaskSchedule)
{
    m_sslServer->setProxy(QNetworkProxy::NoProxy);
    LanLinkProvider::configureSslServer(m_sslServer, deviceId, true);

    QObject::connect(m_sslServer,
                     &QSslServer::pendingConnectionAvailable,
                     this,
                     &FileShareServer::newConnectionEstablished);

    QObject::connect(m_sslServer,
                     &QSslServer::peerVerifyError,
                     this,
                     &FileShareServer::onPeerVerifyError);

    m_tcpPort = MIN_FILE_SHARE_SERVICE_PORT;
    while (!m_sslServer->listen(QHostAddress::Any, m_tcpPort)) {
        m_tcpPort++;
        if (m_tcpPort > MAX_FILE_SHARE_SERVICE_PORT) { // No ports available?
            qCritical(KDECONNECT_PLUGIN_SHARE)
                << "Error opening a port in range" << MIN_FILE_SHARE_SERVICE_PORT << "-"
                << MAX_FILE_SHARE_SERVICE_PORT;
            m_tcpPort = 0;
        }
    }
    m_tcpPort = m_sslServer->serverPort();
    m_sslServer->pauseAccepting();

    QObject::connect(m_sendFilesTaskSchedule.get(),
                     &TaskScheduler::taskStarted,
                     this,
                     &FileShareServer::onTaskStarted);

    QObject::connect(m_sendFilesTaskSchedule.get(),
                     &TaskScheduler::taskFinished,
                     this,
                     &FileShareServer::onTaskFinished);
}

FileShareServer::~FileShareServer()
{
    m_sslServer->close();
}

void FileShareServer::addFileShareTask(const QFileInfo &fileInfo)
{
    NetworkPacket *packet = new NetworkPacket(PACKET_TYPE_SHARE_REQUEST);
    packet->setPayloadSize(fileInfo.size());
    packet->set<QString>(QStringLiteral("filename"), fileInfo.fileName());
    packet->set<qint64>(QStringLiteral("creationTime"), fileInfo.birthTime().toMSecsSinceEpoch());
    packet->set<qint64>(QStringLiteral("lastModified"), fileInfo.lastModified().toMSecsSinceEpoch());
    packet->set<int>(QStringLiteral("numberOfFiles"), 1);
    packet->set<quint64>(QStringLiteral("totalPayloadSize"), fileInfo.size());
    QVariantMap transferInfo;
    transferInfo[QStringLiteral("port")] = m_tcpPort;
    packet->setPayloadTransferInfo(transferInfo);

    PeerFileUploadTask::Ptr task(new PeerFileUploadTask());
    task->setUploadFilePath(fileInfo.absoluteFilePath());

    m_taskToNetworkPackets[task.get()] = packet;
    m_sendFilesTaskSchedule->addTask(task);
}

void FileShareServer::newConnectionEstablished()
{
    while (m_sslServer->hasPendingConnections()) {
        QSslSocket *sslSocket = qobject_cast<QSslSocket *>(m_sslServer->nextPendingConnection());
        if (sslSocket != nullptr) {
            Q_ASSERT(sslSocket->isEncrypted());
            sslSocket->setProxy(QNetworkProxy::NoProxy);

            // sever the relationship with sslserver, make the socket deleted by share pointer
            sslSocket->setParent(nullptr);
            QSharedPointer<QSslSocket> sp(sslSocket);
            m_sslServer->pauseAccepting();
            if (m_currentTask)
                m_currentTask->startUploadFileWithSocket(sp);
        }
    }
}

void FileShareServer::onPeerVerifyError(QSslSocket *socket, const QSslError &error)
{
    if (error.error() != QSslError::SelfSignedCertificate) {
        qCCritical(KDECONNECT_PLUGIN_SHARE) << "Disconnecting due to fatal SSL Error: " << error;
        socket->close();
    } else {
        qCDebug(KDECONNECT_PLUGIN_SHARE) << "Ignoring self-signed cert error";
    }
}

void FileShareServer::onTaskStarted(Task::Ptr task)
{
    auto it = m_taskToNetworkPackets.find(task.get());
    if (it != m_taskToNetworkPackets.end()) {
        PeerFileUploadTask *uploadTask = qobject_cast<PeerFileUploadTask *>(it.key());
        Q_ASSERT(uploadTask != nullptr);
        if (uploadTask != nullptr) {
            m_currentTask = qSharedPointerCast<PeerFileUploadTask>(task);
            auto packet = it.value();
            m_sslServer->resumeAccepting();
            Q_EMIT requestSendPacket(*packet);
            delete packet;
            m_taskToNetworkPackets.erase(it);
        } else {
            Q_ASSERT(0);
        }

    } else {
        Q_ASSERT(0);
    }
}

void FileShareServer::onTaskFinished(Task::Ptr task)
{
    Q_ASSERT(task.get() == m_currentTask.get());
    m_currentTask.reset();
    m_sslServer->pauseAccepting();
}
