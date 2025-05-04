#pragma once

#include "QObjectPtr.h"
#include "kdeconnectcore_export.h"
#include "task.h"

#include <QSslSocket>

class KDECONNECTCORE_EXPORT SocketTask : public Task
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<SocketTask>;

    explicit SocketTask(QObject *parent = nullptr);
    virtual ~SocketTask() override = default;

    void setPeerHostAndPort(const QString &host, quint16 port)
    {
        m_peerHost = host;
        m_peerPort = port;
    }

    void setPeerDeviceId(const QString &deviceId) { m_peerDeviceId = deviceId; }

protected:
    virtual void executeTask() override;

    QSharedPointer<QSslSocket> socket() const { return m_socket; }
    bool isConnected() const;
    void closeSocket();

protected Q_SLOTS:
    virtual void socketConnected();
    virtual void connectError(QAbstractSocket::SocketError socketError);
    virtual void socketDisconnected();
    virtual void socketEncrypted();
    virtual void sslErrors(const QList<QSslError> &errors);
    virtual void dataReceived();

private:
    QSharedPointer<QSslSocket> m_socket;
    QString m_peerHost;
    quint16 m_peerPort = 0;
    QString m_peerDeviceId;
};
