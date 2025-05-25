#include "kdeconnectbufferdownloadtask.h"
#include "core_debug.h"

KdeConnectBufferDownloadTask::KdeConnectBufferDownloadTask(QObject *parent)
    : SocketTask(parent)
    , m_hashCal(QCryptographicHash::Sha1)
{}

QByteArray KdeConnectBufferDownloadTask::downloadedBuffer(QByteArray *sha1Result) const
{
    if (sha1Result != nullptr) {
        sha1Result->assign(m_hashCal.result());
    }

    return m_dlData;
}

void KdeConnectBufferDownloadTask::onAbort()
{
    m_aborted = true;
    closeSocket();
}

void KdeConnectBufferDownloadTask::finisheTask()
{
    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        emitSucceeded();
    }
}

void KdeConnectBufferDownloadTask::socketConnected()
{
    m_connected = true;
}

void KdeConnectBufferDownloadTask::sslErrors(const QList<QSslError> &errors)
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

void KdeConnectBufferDownloadTask::connectError(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        m_errorOccured = true;
        m_errorStr = socket()->errorString();

        qWarning(KDECONNECT_CORE) << "connectError:" << socketError << ", error str:" << m_errorStr;
    }

    if (!m_connected) {
        finisheTask();
    }
}

void KdeConnectBufferDownloadTask::socketDisconnected()
{
    m_connected = false;
    finisheTask();
}

void KdeConnectBufferDownloadTask::socketEncrypted()
{
    m_hashCal.reset();
    m_dlData.clear();
}

void KdeConnectBufferDownloadTask::dataReceived()
{
    auto s = socket();
    auto newData = s->readAll();
    m_dlData.append(newData);
    m_hashCal.addData(newData);
}
