#pragma once

#include "task/sockettask.h"

#include <QByteArray>
#include <QCryptographicHash>

class AlbumArtDownloadTask : public SocketTask
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<AlbumArtDownloadTask>;

    AlbumArtDownloadTask(QObject *parent = nullptr);
    virtual ~AlbumArtDownloadTask() override = default;

    QByteArray albumArtData(QByteArray *sha1Result = nullptr) const;

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
