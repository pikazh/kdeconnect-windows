#pragma once

#include "task/sockettask.h"

#include <QCryptographicHash>
#include <QFile>

class AlbumArtDownloadTask : public SocketTask
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<AlbumArtDownloadTask>;

    AlbumArtDownloadTask(QObject *parent = nullptr);
    virtual ~AlbumArtDownloadTask() override = default;

    void setDownloadDir(const QString &dir);

    QString albumArtFilePath() const;

protected:
    virtual void onAbort() override;

protected Q_SLOTS:
    virtual void connectError(QAbstractSocket::SocketError socketError) override;
    virtual void sslErrors(const QList<QSslError> &errors) override;
    virtual void socketDisconnected() override;
    virtual void socketEncrypted() override;
    virtual void dataReceived() override;

private:
    QFile m_albumArtFile;
    quint64 m_readSize = 0;
    bool m_aborted = false;
    bool m_errorOccured = false;
    QString m_errorStr;
    QString m_albumArtDir;
    QCryptographicHash m_hashCal;
};
