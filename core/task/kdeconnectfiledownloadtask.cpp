#include "kdeconnectfiledownloadtask.h"
#include "core_debug.h"

#include <QFileInfo>

KdeConnectFileDownloadTask::KdeConnectFileDownloadTask(QObject *parent)
    : SocketTask(parent)
    , m_hashCal(QCryptographicHash::Sha1)
{}

QString KdeConnectFileDownloadTask::downloadedFilePath(QByteArray *sha1Result) const
{
    if (sha1Result != nullptr) {
        sha1Result->assign(m_hashCal.result());
    }

    return m_downloadFilePath;
}

void KdeConnectFileDownloadTask::setDownloadFilePath(const QString &filePath)
{
    QString newFilePath = filePath;
    int i = 1;
    while (QFileInfo::exists(newFilePath)) {
        QFileInfo fileInfo(newFilePath);
        newFilePath = fileInfo.absolutePath() + fileInfo.completeBaseName() + QStringLiteral("_");
        newFilePath += QString::number(i++);
        if (!fileInfo.suffix().isEmpty()) {
            newFilePath += QStringLiteral(".");
            newFilePath += fileInfo.suffix();
        }
    }
    m_downloadFilePath = newFilePath;
}

void KdeConnectFileDownloadTask::onAbort()
{
    m_aborted = true;
    closeSocket();
}

void KdeConnectFileDownloadTask::finisheTask()
{
    if (m_aborted || m_errorOccured) {
        if (m_downloadedFile.isOpen()) {
            m_downloadedFile.close();
            m_downloadedFile.remove();
        }
    } else {
        Q_ASSERT(m_downloadedFile.isOpen());
        m_downloadedFile.close();
    }

    if (m_aborted) {
        emitAborted();
    } else if (m_errorOccured) {
        emitFailed(m_errorStr);
    } else {
        emitSucceeded();
    }
}

void KdeConnectFileDownloadTask::socketConnected()
{
    m_connected = true;
}

void KdeConnectFileDownloadTask::sslErrors(const QList<QSslError> &errors)
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

void KdeConnectFileDownloadTask::connectError(QAbstractSocket::SocketError socketError)
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

void KdeConnectFileDownloadTask::socketDisconnected()
{
    m_connected = false;
    finisheTask();
}

void KdeConnectFileDownloadTask::socketEncrypted()
{
    m_hashCal.reset();

    m_downloadedFile.setFileName(m_downloadFilePath);
    if (!m_downloadedFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate)) {
        qCritical(KDECONNECT_CORE)
            << "open file" << m_downloadFilePath << "for downloading failed.";

        m_errorOccured = true;
        m_errorStr = "Can not open file to write";

        closeSocket();
    }
}

void KdeConnectFileDownloadTask::dataReceived()
{
    auto s = socket();
    auto newData = s->readAll();
    if (newData.size() != m_downloadedFile.write(newData)) {
        m_errorOccured = true;
        m_errorStr = "Can not write buffer to the specified file";

        qCritical(KDECONNECT_CORE) << m_errorStr;

        closeSocket();
        return;
    }

    m_hashCal.addData(newData);
}
