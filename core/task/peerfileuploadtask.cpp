#include "peerfileuploadtask.h"

PeerFileUploadTask::PeerFileUploadTask(QObject *parent)
    : Task(parent)
{
    QObject::connect(this, &PeerFileUploadTask::started, this, [this]() {
        setTaskStatus(TaskStatus::WaitingIncomeConnection);
    });
}

void PeerFileUploadTask::startUploadFileWithSocket(shared_qobject_ptr<QSslSocket> socket)
{
    Q_ASSERT(socket && socket->isEncrypted());
    setTaskStatus(TaskStatus::Transfering);
    m_socket = socket;

    connect(m_socket.get(),
            &QSslSocket::encryptedBytesWritten,
            this,
            &PeerFileUploadTask::encryptedBytesWritten);
    connect(m_socket.get(),
            &QSslSocket::disconnected,
            this,
            &PeerFileUploadTask::socketDisconnected);

    Q_ASSERT(!m_uploadFile.fileName().isEmpty() && !m_uploadFile.isOpen());
    if (!m_uploadFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::ExistingOnly)) {
        QString errStr = tr("Can not open upload file:").append(filePath);
        emitFailed(errStr);
    }
}

void PeerFileUploadTask::setUploadFile(const QString &filePath)
{
    m_uploadFile.setFileName(filePath);
}
