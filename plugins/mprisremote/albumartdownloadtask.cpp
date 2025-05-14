#include "albumartdownloadtask.h"
#include "plugin_mprisremote_debug.h"

#include <QDir>
#include <QUuid>

AlbumArtDownloadTask::AlbumArtDownloadTask(QObject *parent)
    : SocketTask(parent)
    , m_hashCal(QCryptographicHash::Sha1)
{
}

void AlbumArtDownloadTask::setDownloadDir(const QString &dir)
{
    m_albumArtDir = dir;
}

QString AlbumArtDownloadTask::albumArtFilePath() const
{
    return m_albumArtFile.fileName();
}

void AlbumArtDownloadTask::onAbort()
{
    m_aborted = true;
    closeSocket();
}

void AlbumArtDownloadTask::finisheTask()
{
    if (m_albumArtFile.isOpen()) {
        m_albumArtFile.close();

        if (m_aborted || m_errorOccured) {
            m_albumArtFile.remove();
        }
    }

    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        QString newFileName = QString("%1_%2").arg(m_hashCal.result().toHex()).arg(m_readSize);
        QDir cacheDir(m_albumArtDir);
        if (!cacheDir.exists(newFileName) || cacheDir.remove(newFileName)) {
            m_albumArtFile.rename(m_albumArtDir + QDir::separator() + newFileName);
        }

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
    m_readSize = 0;

    m_albumArtFile.setFileName(m_albumArtDir + QDir::separator()
                               + QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (!m_albumArtFile.open(QIODevice::WriteOnly)) {
        m_errorOccured = true;
        m_errorStr = QString(tr("Can not open file %1 for writing ablum art."))
                         .arg(m_albumArtFile.fileName());
        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE) << m_errorStr;

        closeSocket();
    }
}

void AlbumArtDownloadTask::dataReceived()
{
    auto s = socket();
    QByteArray bytes = s->readAll();
    m_readSize += bytes.length();
    m_albumArtFile.write(bytes);
    m_hashCal.addData(bytes);
}
