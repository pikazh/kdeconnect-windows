#pragma once

#include "kdeconnectcore_export.h"

#include "QObjectPtr.h"
#include "task.h"

#include <QFile>
#include <QSslSocket>
#include <QTimer>

#include <chrono>

class KDECONNECTCORE_EXPORT PeerFileUploadTask : public Task
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<PeerFileUploadTask>;
    explicit PeerFileUploadTask(QObject *parent = nullptr);

    void startUploadFileWithSocket(shared_qobject_ptr<QSslSocket> socket);
    void setUploadFilePath(const QString &filePath);
    QString uploadFilePath();
    qint64 uploadFileSize();

protected:
    virtual void onAbort() override;
    virtual void executeTask() override;

    void finisheTask();
    void updateProgressIntervally(qint64 current, qint64 total);
    void sendFileData();

    void verifyUploadedSize();

protected Q_SLOTS:
    void encryptedBytesWritten(qint64 written);
    void socketDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

    void onPeerConnectTimeOut();

private:
    shared_qobject_ptr<QSslSocket> m_socket;
    QFile m_uploadFile;
    qint64 m_fileSize = -1;
    qint64 m_bytesWritten = 0;

    bool m_errorOccured = false;
    QString m_errorStr;
    bool m_aborted = false;

    std::chrono::time_point<std::chrono::system_clock> m_lastUpdateProgressTimePoint;
    QTimer *m_peerTimeoutTimer;
};
