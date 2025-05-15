#include "albumartdownloadtask.h"
#include "plugin_mprisremote_debug.h"

AlbumArtDownloadTask::AlbumArtDownloadTask(QObject *parent)
    : SocketTask(parent)
    , m_hashCal(QCryptographicHash::Sha1)
{
}

QByteArray AlbumArtDownloadTask::albumArtData(QByteArray *sha1Result) const
{
    if (sha1Result != nullptr) {
        sha1Result->assign(m_hashCal.result());
    }

    return m_dlData;
}

void AlbumArtDownloadTask::onAbort()
{
    m_aborted = true;
    closeSocket();
}

void AlbumArtDownloadTask::finisheTask()
{
    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        emitSucceeded();
    }
}

void AlbumArtDownloadTask::socketConnected()
{
    m_connected = true;
}

void AlbumArtDownloadTask::sslErrors(const QList<QSslError> &errors)
{
    for (const QSslError &error : errors) {
        if (error.error() != QSslError::SelfSignedCertificate) {
            qCCritical(KDECONNECT_PLUGIN_MPRISREMOTE)
                << "Disconnecting due to fatal SSL Error: " << error;
            m_errorStr = error.errorString();
            m_errorOccured = true;
            break;
        } else {
            qCDebug(KDECONNECT_PLUGIN_MPRISREMOTE) << "Ignoring self-signed cert error";
        }
    }

    if (m_errorOccured) {
        closeSocket();
    }
}

void AlbumArtDownloadTask::connectError(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        m_errorOccured = true;
        m_errorStr = socket()->errorString();

        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "connectError:" << socketError << ", error str:" << m_errorStr;
    }

    if (!m_connected) {
        finisheTask();
    }
}

void AlbumArtDownloadTask::socketDisconnected()
{
    m_connected = false;
    finisheTask();
}

void AlbumArtDownloadTask::socketEncrypted()
{
    m_hashCal.reset();
    m_dlData.clear();
}

void AlbumArtDownloadTask::dataReceived()
{
    auto s = socket();
    auto newData = s->readAll();
    m_dlData.append(newData);
    m_hashCal.addData(newData);
}
