#include "sockettask.h"
#include "backends/lan/lanlinkprovider.h"
#include "core_debug.h"

SocketTask::SocketTask(QObject *parent)
    : Task(parent)
    , m_socket(new QSslSocket)
{
    auto socket = m_socket.get();
    connect(socket, &QAbstractSocket::connected, this, &SocketTask::socketConnected);
    connect(socket, &QAbstractSocket::errorOccurred, this, &SocketTask::connectError);
    connect(socket, &QAbstractSocket::disconnected, this, &SocketTask::socketDisconnected);
    connect(socket, &QSslSocket::encrypted, this, &SocketTask::socketEncrypted);
    connect(socket, &QSslSocket::sslErrors, this, &SocketTask::sslErrors);
    connect(socket, &QAbstractSocket::readyRead, this, &SocketTask::dataReceived);
}

void SocketTask::executeTask()
{
    if (m_peerDeviceId.isEmpty() || m_peerHost.isEmpty() || m_peerPort == 0) {
        emitFailed(tr("Peer info is not set!"));
        return;
    }

    LanLinkProvider::configureSslSocket(m_socket.get(), m_peerDeviceId, true);
    m_socket->connectToHostEncrypted(m_peerHost, m_peerPort);
}

bool SocketTask::isConnected() const
{
    return (m_socket.get()->state() == QAbstractSocket::ConnectedState) ? true : false;
}

void SocketTask::closeSocket()
{
    m_socket->disconnectFromHost();
}

void SocketTask::socketConnected()
{
    //qDebug(KDECONNECT_CORE) << "socket connected";
}

void SocketTask::connectError(QAbstractSocket::SocketError socketError)
{
    //qWarning(KDECONNECT_CORE) << "error occured:" << m_socket->errorString();

    //emitFailed(m_socket->errorString());
}

void SocketTask::socketDisconnected() {}

void SocketTask::sslErrors(const QList<QSslError> &errors) {}

void SocketTask::dataReceived() {}

void SocketTask::socketEncrypted() {}
