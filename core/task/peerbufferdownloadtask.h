#pragma once

#include "peersslsockettask.h"

#include <QByteArray>
#include <QCryptographicHash>

class KDECONNECTCORE_EXPORT PeerBufferDownloadTask : public PeerSSLSocketTask
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<PeerBufferDownloadTask>;

    PeerBufferDownloadTask(QObject *parent = nullptr);
    virtual ~PeerBufferDownloadTask() override = default;

    QByteArray downloadedBuffer(QByteArray *sha1Result = nullptr) const;

protected:
    virtual void onAbort() override;

    void finisheTask();

protected Q_SLOTS:

    virtual void connectError(QAbstractSocket::SocketError socketError) override;
    virtual void sslErrors(const QList<QSslError> &errors) override;
    virtual void socketDisconnected() override;
    virtual void socketEncrypted() override;
    virtual void dataReceived() override;

private:
    bool m_aborted = false;
    bool m_errorOccured = false;

    QString m_errorStr;
    QByteArray m_dlData;
    QCryptographicHash m_hashCal;
    qint64 m_downloadedSize = 0;
};
