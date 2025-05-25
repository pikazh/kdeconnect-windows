#pragma once

#include "sockettask.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>

class KDECONNECTCORE_EXPORT KdeConnectFileDownloadTask : public SocketTask
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<KdeConnectFileDownloadTask>;

    KdeConnectFileDownloadTask(QObject *parent = nullptr);
    virtual ~KdeConnectFileDownloadTask() override = default;

    QString downloadedFilePath(QByteArray *sha1Result = nullptr) const;
    void setDownloadFilePath(const QString &filePath);

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
    QCryptographicHash m_hashCal;
    QFile m_downloadedFile;
    QString m_downloadFilePath;
};
