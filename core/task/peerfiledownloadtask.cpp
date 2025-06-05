#include "peerfiledownloadtask.h"
#include "core_debug.h"

#include <QDir>
#include <QFileInfo>

const int bufferSize = 8 * 1024 * 1024;
const int padSize = 1024 * 1024;

PeerFileDownloadTask::PeerFileDownloadTask(QObject *parent)
    : PeerSSLSocketTask(parent)
    , m_hashCal(QCryptographicHash::Sha1)
{}

QString PeerFileDownloadTask::downloadFilePath() const
{
    return m_downloadFilePath;
}

void PeerFileDownloadTask::setDownloadFilePath(const QString &filePath)
{
    QString newFilePath = filePath;
    int i = 1;
    QFileInfo fileInfo(filePath);
    while (QFileInfo::exists(newFilePath)) {
        newFilePath = fileInfo.absolutePath() + QDir::separator() + fileInfo.completeBaseName()
                      + QStringLiteral("(");
        newFilePath.append(QString::number(i++)).append(QStringLiteral(")"));
        if (!fileInfo.suffix().isEmpty()) {
            newFilePath += QStringLiteral(".");
            newFilePath += fileInfo.suffix();
        }
    }
    m_downloadFilePath = newFilePath;
}

void PeerFileDownloadTask::onAbort()
{
    //Q_ASSERT(isRunning());
    m_aborted = true;
    closeSocket();
}

void PeerFileDownloadTask::finisheTask()
{
    if (m_aborted || m_errorOccured) {
        if (m_downloadedFile.isOpen()) {
            m_downloadedFile.close();
            m_downloadedFile.remove();
        }
    } else {
        Q_ASSERT(m_downloadedFile.isOpen());
        if (!m_buffer.isEmpty()) {
            if (m_buffer.size() != m_downloadedFile.write(m_buffer)) {
                m_errorOccured = true;
                m_errorStr = tr("Can not write buffer to the specified file");

                qCritical(KDECONNECT_CORE) << m_errorStr;

            } else {
                m_hashCal.addData(m_buffer);
                m_buffer.clear();
            }
        }

        m_downloadedFile.close();

        verifyDownloadedSize();
    }

    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        emitSucceeded();
    }
}

void PeerFileDownloadTask::verifyDownloadedSize()
{
    Q_ASSERT(!m_aborted && !m_errorOccured);
    if (m_downloadedSize != contentSize()) {
        m_errorOccured = true;
        m_errorStr = tr("Connection closed before download is complete");
    }
}

void PeerFileDownloadTask::sslErrors(const QList<QSslError> &errors)
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

void PeerFileDownloadTask::connectError(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        m_errorOccured = true;
        m_errorStr = socket()->errorString();

        qWarning(KDECONNECT_CORE) << describe() << "connectError:" << socketError
                                  << ", error str:" << m_errorStr;
    }

    if (socketConnectState() != SocketState::Connected) {
        finisheTask();
    }
}

void PeerFileDownloadTask::socketDisconnected()
{
    finisheTask();
}

void PeerFileDownloadTask::socketEncrypted()
{
    setTaskStatus(TaskStatus::Transfering);
    m_hashCal.reset();
    m_buffer.clear();
    m_buffer.reserve(bufferSize + padSize);

    m_downloadedSize = 0;

    m_downloadedFile.setFileName(m_downloadFilePath);
    if (!m_downloadedFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate)) {
        qCritical(KDECONNECT_CORE)
            << "open file" << m_downloadFilePath << "for downloading failed.";

        m_errorOccured = true;
        m_errorStr = tr("Can not open file to write");

        closeSocket();
    }
}

void PeerFileDownloadTask::dataReceived()
{
    auto s = socket();
    m_downloadedSize += s->bytesAvailable();
    updateProgressIntervally(m_downloadedSize, contentSize());
    m_buffer.append(s->readAll());

    if (m_buffer.size() >= bufferSize) {
        if (m_buffer.size() != m_downloadedFile.write(m_buffer)) {
            m_errorOccured = true;
            m_errorStr = tr("Can not write buffer to the specified file");

            qCritical(KDECONNECT_CORE) << m_errorStr;

            closeSocket();

        } else {
            m_hashCal.addData(m_buffer);
            m_buffer.clear();
            m_buffer.reserve(bufferSize + padSize);
        }
    }
}
