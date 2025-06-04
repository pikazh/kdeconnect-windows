#pragma once

#include "peersslsockettask.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>

class KDECONNECTCORE_EXPORT PeerFileDownloadTask : public PeerSSLSocketTask
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<PeerFileDownloadTask>;

    PeerFileDownloadTask(QObject *parent = nullptr);
    virtual ~PeerFileDownloadTask() override = default;

    QString downloadFilePath() const;
    void setDownloadFilePath(const QString &filePath);

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
    QCryptographicHash m_hashCal;
    QFile m_downloadedFile;
    QString m_downloadFilePath;
    QByteArray m_buffer;
    qint64 m_downloadedSize = 0;
};
