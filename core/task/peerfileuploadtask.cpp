#include "peerfileuploadtask.h"
#include "core_debug.h"

using namespace std::chrono_literals;

const int connectTimeOutTimeInMs = 20 * 1000;

PeerFileUploadTask::PeerFileUploadTask(QObject *parent)
    : Task(parent)
    , m_lastUpdateProgressTimePoint(std::chrono::system_clock::now())
    , m_peerTimeoutTimer(new QTimer(this))
{
    m_peerTimeoutTimer->setSingleShot(true);
    m_peerTimeoutTimer->setInterval(connectTimeOutTimeInMs);
    QObject::connect(m_peerTimeoutTimer,
                     &QTimer::timeout,
                     this,
                     &PeerFileUploadTask::onPeerConnectTimeOut);

    QObject::connect(this, &PeerFileUploadTask::started, this, [this]() {
        setTaskStatus(TaskStatus::WaitingIncomeConnection);
        m_peerTimeoutTimer->start();
    });
}

void PeerFileUploadTask::startUploadFileWithSocket(shared_qobject_ptr<QSslSocket> socket)
{
    if (isRunning() && taskStatus() == TaskStatus::WaitingIncomeConnection) {
        Q_ASSERT(socket && socket->isEncrypted());
        m_peerTimeoutTimer->stop();
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
        connect(m_socket.get(),
                &QSslSocket::errorOccurred,
                this,
                &PeerFileUploadTask::onErrorOccurred);

        Q_ASSERT(!m_uploadFile.fileName().isEmpty() && !m_uploadFile.isOpen());
        if (!m_uploadFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::ExistingOnly)) {
            m_errorOccured = true;
            m_errorStr = tr("Can not open upload file:").append(m_uploadFile.fileName());

            m_socket->close();
        } else {
            m_fileSize = m_uploadFile.size();
            sendFileData();
        }
    }
}

void PeerFileUploadTask::updateProgressIntervally(qint64 current, qint64 total)
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> dur = now - m_lastUpdateProgressTimePoint;
    if (dur > 500ms) {
        setProgress(current, total);
        m_lastUpdateProgressTimePoint = now;
    }
}

void PeerFileUploadTask::setUploadFilePath(const QString &filePath)
{
    m_uploadFile.setFileName(filePath);
}

QString PeerFileUploadTask::uploadFilePath()
{
    return m_uploadFile.fileName();
}

qint64 PeerFileUploadTask::uploadFileSize()
{
    return m_uploadFile.size();
}

void PeerFileUploadTask::onAbort()
{
    m_aborted = true;
    if (taskStatus() == TaskStatus::WaitingIncomeConnection) {
        m_peerTimeoutTimer->stop();
        emitAborted();
    } else if (taskStatus() == TaskStatus::Transfering) {
        m_socket->close();
    }
}

void PeerFileUploadTask::executeTask() {}

void PeerFileUploadTask::finisheTask()
{
    if (m_uploadFile.isOpen()) {
        m_uploadFile.close();
    }

    if (!m_aborted && !m_errorOccured) {
        verifyUploadedSize();
    }

    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        emitSucceeded();
    }
}

void PeerFileUploadTask::sendFileData()
{
    const int readSize = 1024;
    QByteArray buf = m_uploadFile.read(readSize);
    m_bytesWritten += buf.size();
    if (!buf.isEmpty()) {
        m_socket->write(buf);
    } else {
        m_socket->disconnectFromHost();
    }
}

void PeerFileUploadTask::verifyUploadedSize()
{
    Q_ASSERT(!m_aborted && !m_errorOccured);
    if (m_bytesWritten != m_fileSize) {
        m_errorOccured = true;
        m_errorStr = tr("Connection closed before upload is complete");
    }
}

void PeerFileUploadTask::encryptedBytesWritten(qint64 written)
{
    updateProgressIntervally(m_bytesWritten, m_fileSize);
    if (m_socket->encryptedBytesToWrite() == 0) {
        sendFileData();
    }
}

void PeerFileUploadTask::socketDisconnected()
{
    finisheTask();
}

void PeerFileUploadTask::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    m_errorOccured = true;
    m_errorStr = m_socket->errorString();

    qWarning(KDECONNECT_CORE) << describe() << "network error occured while uploading file"
                              << uploadFilePath() << ":" << socketError
                              << ", error str:" << m_errorStr;
}

void PeerFileUploadTask::onPeerConnectTimeOut()
{
    m_errorOccured = true;
    m_errorStr = tr("Connection times out");
    emitFailed(m_errorStr);
}
