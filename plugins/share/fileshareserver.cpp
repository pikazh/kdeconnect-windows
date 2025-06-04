#include "fileshareserver.h"
#include "common.h"
#include "plugin_share_debug.h"

#include "core/backends/lan/lanlinkprovider.h"

#include <QNetworkProxy>

#define MIN_FILE_SHARE_SERVICE_PORT (2000)
#define MAX_FILE_SHARE_SERVICE_PORT (2030)

FileShareServer::FileShareServer(QSharedPointer<TaskScheduler> sendFilesTaskSchedule,
                                 QString deviceId,
                                 QObject *parent)
    : QObject{parent}
    , m_sslServer(new QSslServer(this))
    , m_deviceId(deviceId)
    , m_sendFilesTaskSchedule(sendFilesTaskSchedule)
{
    m_sslServer->setProxy(QNetworkProxy::NoProxy);
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
}

FileShareServer::~FileShareServer()
{
    m_sslServer->close();
}

void FileShareServer::addFileShareTask(const QFileInfo &fileInfo)
{
    NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
    packet.setPayloadSize(fileInfo.size());
    packet.set<QString>(QStringLiteral("filename"), fileInfo.fileName());
    packet.set<qint64>(QStringLiteral("creationTime"), fileInfo.birthTime().toMSecsSinceEpoch());
    packet.set<qint64>(QStringLiteral("lastModified"), fileInfo.lastModified().toMSecsSinceEpoch());
}

void FileShareServer::newConnectionEstablished()
{
    while (m_sslServer->hasPendingConnections()) {
        QSslSocket *sslSocket = qobject_cast<QSslSocket *>(m_sslServer->nextPendingConnection());
        if (sslSocket != nullptr) {
            sslSocket->setProxy(QNetworkProxy::NoProxy);
            QObject::connect(sslSocket,
                             &QSslSocket::disconnected,
                             sslSocket,
                             &QSslSocket::deleteLater);

            QObject::connect(sslSocket,
                             &QSslSocket::errorOccurred,
                             this,
                             &FileShareServer::socketErrorOccured);

            QObject::connect(sslSocket,
                             &QSslSocket::encrypted,
                             this,
                             &FileShareServer::socketEncrypted);

            LanLinkProvider::configureSslSocket(sslSocket, m_deviceId, true);
            sslSocket->startServerEncryption();
        }
    }
}

void FileShareServer::onPeerVerifyError(QSslSocket *socket, const QSslError &error)
{
    if (error.error() != QSslError::SelfSignedCertificate) {
        qCCritical(KDECONNECT_PLUGIN_SHARE) << "Disconnecting due to fatal SSL Error: " << error;
        socket->disconnectFromHost();
    } else {
        qCDebug(KDECONNECT_PLUGIN_SHARE) << "Ignoring self-signed cert error";
    }
}

void FileShareServer::socketErrorOccured(QAbstractSocket::SocketError error)
{
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    if (socket == nullptr)
        return;

    qCritical(KDECONNECT_PLUGIN_SHARE) << "socket error:" << error;
}

void FileShareServer::socketEncrypted() {}
