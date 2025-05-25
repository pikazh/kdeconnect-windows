#pragma once

#include "sockettask.h"

#include <QByteArray>
#include <QCryptographicHash>

class KDECONNECTCORE_EXPORT KdeConnectBufferDownloadTask : public SocketTask
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<KdeConnectBufferDownloadTask>;

    KdeConnectBufferDownloadTask(QObject *parent = nullptr);
    virtual ~KdeConnectBufferDownloadTask() override = default;

    QByteArray downloadedBuffer(QByteArray *sha1Result = nullptr) const;

protected:
    virtual void onAbort() override;

    void finisheTask();

protected Q_SLOTS:
    virtual void socketConnected() override;
    virtual void connectError(QAbstractSocket::SocketError socketError) override;
    virtual void sslErrors(const QList<QSslError> &errors) override;
    virtual void socketDisconnected() override;
    virtual void socketEncrypted() override;
    virtual void dataReceived() override;

private:
    bool m_aborted = false;
    bool m_errorOccured = false;
    bool m_connected = false;
    QString m_errorStr;
    QByteArray m_dlData;
    QCryptographicHash m_hashCal;
};
