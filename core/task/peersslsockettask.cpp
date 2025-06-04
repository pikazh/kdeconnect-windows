#include "peersslsockettask.h"
#include "backends/lan/lanlinkprovider.h"
#include "core_debug.h"

#include <QNetworkProxy>

using namespace std::chrono_literals;

PeerSSLSocketTask::PeerSSLSocketTask(QObject *parent)
    : Task(parent)
    , m_socket(new QSslSocket)
    , m_lastUpdateProgressTimePoint(std::chrono::system_clock::now())
{
    m_socket->setProxy(QNetworkProxy::NoProxy);
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

void PeerSSLSocketTask::updateProgressIntervally(qint64 current, qint64 total)
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> dur = now - m_lastUpdateProgressTimePoint;
    if (dur > 500ms) {
        setProgress(current, total);
        m_lastUpdateProgressTimePoint = now;
    }
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
