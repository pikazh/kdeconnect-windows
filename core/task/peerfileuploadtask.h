#pragma once

#include "kdeconnectcore_export.h"

#include "QObjectPtr.h"
#include "task.h"

#include <QFile>
#include <QSslSocket>

#include <chrono>

class KDECONNECTCORE_EXPORT PeerFileUploadTask : public Task
{
    Q_OBJECT
public:
    using Ptr = shared_qobject_ptr<PeerFileUploadTask>;
    explicit PeerFileUploadTask(QObject *parent = nullptr);

    void startUploadFileWithSocket(shared_qobject_ptr<QSslSocket> socket);
    void setUploadFile(const QString &filePath);

protected:
    void encryptedBytesWritten(qint64 written);
    void socketDisconnected();

private:
    shared_qobject_ptr<QSslSocket> m_socket;
    QFile m_uploadFile;
};
