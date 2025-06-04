#pragma once

#include "kdeconnectcore_export.h"

#include "QObjectPtr.h"
#include "task.h"

#include <QSslSocket>

#include <chrono>

class KDECONNECTCORE_EXPORT PeerSSLSocketTask : public Task
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<PeerSSLSocketTask>;

    explicit PeerSSLSocketTask(QObject *parent = nullptr);
    virtual ~PeerSSLSocketTask() override = default;

    void setPeerHostAndPort(const QString &host, quint16 port)
    {
        m_peerHost = host;
        m_peerPort = port;
    }

    void setPeerDeviceId(const QString &deviceId) { m_peerDeviceId = deviceId; }
    void setContentSize(qint64 size) { m_contentSize = size; }
    qint64 contentSize() const { return m_contentSize; }

protected:
    enum class SocketState {
        NotConnected = 0,
        Connecting,
        Connected,
    };

    virtual void executeTask() override;

    void updateProgressIntervally(qint64 current, qint64 total);

    QSharedPointer<QSslSocket> socket() const { return m_socket; }
    SocketState socketConnectState() const;
    void closeSocket();

protected Q_SLOTS:
    virtual void socketConnected();
    virtual void connectError(QAbstractSocket::SocketError socketError);
    virtual void socketDisconnected();
    virtual void socketEncrypted();
    virtual void sslErrors(const QList<QSslError> &errors);
    virtual void dataReceived();

private:
    shared_qobject_ptr<QSslSocket> m_socket;
    QString m_peerHost;
    quint16 m_peerPort = 0;
    QString m_peerDeviceId;
    qint64 m_contentSize = -1;
    std::chrono::time_point<std::chrono::system_clock> m_lastUpdateProgressTimePoint;
};
