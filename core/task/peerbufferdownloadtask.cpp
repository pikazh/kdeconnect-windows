#include "peerbufferdownloadtask.h"
#include "core_debug.h"

PeerBufferDownloadTask::PeerBufferDownloadTask(QObject *parent)
    : PeerSSLSocketTask(parent)
    , m_hashCal(QCryptographicHash::Sha1)
{}

QByteArray PeerBufferDownloadTask::downloadedBuffer(QByteArray *sha1Result) const
{
    if (sha1Result != nullptr) {
        sha1Result->assign(m_hashCal.result());
    }

    return m_dlData;
}

void PeerBufferDownloadTask::onAbort()
{
    //Q_ASSERT(isRunning());
    m_aborted = true;
    closeSocket();
}

void PeerBufferDownloadTask::finisheTask()
{
    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        emitSucceeded();
    }
}

void PeerBufferDownloadTask::sslErrors(const QList<QSslError> &errors)
{
    for (const QSslError &error : errors) {
        if (error.error() != QSslError::SelfSignedCertificate) {
            qCCritical(KDECONNECT_CORE) << "Disconnecting due to fatal SSL Error: " << error;
            m_errorStr = error.errorString();
            m_errorOccured = true;
            break;
        } else {
            qCDebug(KDECONNECT_CORE) << "Ignoring self-signed cert error";
        }
    }

    if (m_errorOccured) {
        closeSocket();
    }
}

void PeerBufferDownloadTask::connectError(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        m_errorOccured = true;
        m_errorStr = socket()->errorString();

        qWarning(KDECONNECT_CORE) << "connectError:" << socketError << ", error str:" << m_errorStr;
    }

    if (socketConnectState() != SocketState::Connected) {
        finisheTask();
    }
}

void PeerBufferDownloadTask::socketDisconnected()
{
    finisheTask();
}

void PeerBufferDownloadTask::socketEncrypted()
{
    setTaskStatus(TaskStatus::Transfering);
    m_hashCal.reset();
    m_dlData.clear();
    m_downloadedSize = 0;
}

void PeerBufferDownloadTask::dataReceived()
{
    auto s = socket();
    auto newData = s->readAll();
    m_dlData.append(newData);
    m_hashCal.addData(newData);
    m_downloadedSize += newData.size();

    updateProgressIntervally(m_downloadedSize, contentSize());
}
