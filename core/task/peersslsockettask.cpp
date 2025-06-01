#include "peersslsockettask.h"
#include "backends/lan/lanlinkprovider.h"
#include "core_debug.h"

PeerSSLSocketTask::PeerSSLSocketTask(QObject *parent)
    : Task(parent)
    , m_socket(new QSslSocket)
{
    auto socket = m_socket.get();
    connect(socket, &QAbstractSocket::connected, this, &PeerSSLSocketTask::socketConnected);
    connect(socket, &QAbstractSocket::errorOccurred, this, &PeerSSLSocketTask::connectError);
    connect(socket, &QAbstractSocket::disconnected, this, &PeerSSLSocketTask::socketDisconnected);
    connect(socket, &QSslSocket::encrypted, this, &PeerSSLSocketTask::socketEncrypted);
    connect(socket, &QSslSocket::sslErrors, this, &PeerSSLSocketTask::sslErrors);
    connect(socket, &QAbstractSocket::readyRead, this, &PeerSSLSocketTask::dataReceived);
}

void PeerSSLSocketTask::executeTask()
{
    if (m_peerDeviceId.isEmpty() || m_peerHost.isEmpty() || m_peerPort == 0) {
        emitFailed(tr("Peer info is not set!"));
        return;
    }

    LanLinkProvider::configureSslSocket(m_socket.get(), m_peerDeviceId, true);
    m_socket->connectToHostEncrypted(m_peerHost, m_peerPort);
    setTaskStatus(TaskStatus::ConnectingToPeer);
}

PeerSSLSocketTask::SocketState PeerSSLSocketTask::socketConnectState() const
{
    // assume socket is client side socket
    QAbstractSocket::SocketState socketState = m_socket.get()->state();
    if (socketState == QAbstractSocket::ConnectedState) {
        return SocketState::Connected;
    } else if (socketState == QAbstractSocket::UnconnectedState) {
        return SocketState::NotConnected;
    } else {
        return SocketState::Connecting;
    }
}

void PeerSSLSocketTask::closeSocket()
{
    m_socket->disconnectFromHost();
}

void PeerSSLSocketTask::socketConnected()
{
    //qDebug(KDECONNECT_CORE) << "socket connected";
}

void PeerSSLSocketTask::connectError(QAbstractSocket::SocketError socketError)
{
    //qWarning(KDECONNECT_CORE) << "error occured:" << m_socket->errorString();

    //emitFailed(m_socket->errorString());
}

void PeerSSLSocketTask::socketDisconnected() {}

void PeerSSLSocketTask::sslErrors(const QList<QSslError> &errors) {}

void PeerSSLSocketTask::dataReceived() {}

void PeerSSLSocketTask::socketEncrypted() {}
