/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "shareplugin.h"
#include "common.h"
#include "plugin_share_debug.h"

#include "core/plugins/pluginfactory.h"
#include "core/task/peerfiledownloadtask.h"
#include "core/task/peerfileuploadtask.h"

#include "notification.h"

#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QTemporaryFile>

K_PLUGIN_CLASS_WITH_JSON(SharePlugin, "kdeconnect_share.json")

SharePlugin::SharePlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_recvFilesTaskSchedule(new TaskScheduler(nullptr, 1))
    , m_sendFilesTaskSchedule(new TaskScheduler(nullptr, 1))
    , m_transferHistoryManager(new TransferHistoryManager(device()->id()))
{
    m_fileShareServer = new FileShareServer(m_sendFilesTaskSchedule, device()->id(), this);
    Q_ASSERT(m_tempDir.isValid());

    QObject::connect(config().get(),
                     &KdeConnectPluginConfig::configChanged,
                     this,
                     &SharePlugin::reloadConfig);

    QObject::connect(m_fileShareServer,
                     &FileShareServer::requestSendPacket,
                     this,
                     [this](NetworkPacket &packet) { sendPacket(packet); });

    reloadConfig();

    QObject::connect(m_recvFilesTaskSchedule.get(),
                     &TaskScheduler::taskFinished,
                     this,
                     &SharePlugin::onTaskFinished);

    QObject::connect(m_sendFilesTaskSchedule.get(),
                     &TaskScheduler::taskFinished,
                     this,
                     &SharePlugin::onTaskFinished);

    m_recvFilesTaskSchedule->start();
    m_sendFilesTaskSchedule->start();
}

SharePlugin::~SharePlugin()
{
    m_recvFilesTaskSchedule->stop();
    m_sendFilesTaskSchedule->stop();
}

void SharePlugin::receivePacket(const NetworkPacket &np)
{
    if (np.has(QStringLiteral("filename"))) {
        handleSharedFile(np);
    } else if (np.has(QStringLiteral("text"))) {
        handleSharedText(np);
    } else if (np.has(QStringLiteral("url"))) {
        QUrl url = QUrl::fromEncoded(np.get<QByteArray>(QStringLiteral("url")));
        QDesktopServices::openUrl(url);
        //Q_EMIT shareReceived(url.toString());
    }
}

void SharePlugin::reloadConfig()
{
    const QString defaultDownloadPath = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
    QString dlPath
        = config()->getString(QStringLiteral("incoming_path"), QLatin1StringView("")).trimmed();
    if (!dlPath.isEmpty()) {
        QFileInfo fileInfo(dlPath);
        if (fileInfo.isAbsolute() && QDir().mkpath(dlPath)) {
            m_saveFileDir = dlPath;
            return;
        }
    }

    m_saveFileDir = defaultDownloadPath;
}

void SharePlugin::onTaskFinished(Task::Ptr task)
{
    if (PeerFileDownloadTask *dlTask = qobject_cast<PeerFileDownloadTask *>(task.get());
        dlTask != nullptr) {
        m_transferHistoryManager->addHistory(TransferHistoryRecord::TransferType::Receiving,
                                             dlTask->downloadFilePath(),
                                             QDateTime::currentSecsSinceEpoch(),
                                             taskResult(task.get()),
                                             dlTask->failedReasson());
    } else if (PeerFileUploadTask *uploadTask = qobject_cast<PeerFileUploadTask *>(task.get());
               uploadTask != nullptr) {
        m_transferHistoryManager->addHistory(TransferHistoryRecord::TransferType::Sending,
                                             uploadTask->uploadFilePath(),
                                             QDateTime::currentSecsSinceEpoch(),
                                             taskResult(task.get()),
                                             uploadTask->failedReasson());
    }
}

void SharePlugin::handleSharedText(const NetworkPacket &np)
{
    QString text = np.get<QString>(QStringLiteral("text"));

    QGuiApplication::clipboard()->setText(text);

    QString title = tr("Shared text from %1 copied to clipboard").arg(device()->name());
    auto notif = Notification::exec(QStringLiteral("KDE Connect"), title);
    auto *textEditorAction = notif->addAction(tr("Open in Text Editor"));
    auto openTextEditor = [this, text] {
        QTemporaryFile tmpFile;
        tmpFile.setFileTemplate(m_tempDir.path() + QDir::separator()
                                + QStringLiteral("kdeconnect-XXXXXX.txt"));
        tmpFile.setAutoRemove(false);
        tmpFile.open();
        tmpFile.write(text.toUtf8());
        tmpFile.close();

        const QString fileName = tmpFile.fileName();
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    };
    connect(textEditorAction, &NotificationAction::activated, this, openTextEditor);

    //Q_EMIT shareReceived(fileName);
}

void SharePlugin::handleSharedFile(const NetworkPacket &np)
{
    if (np.hasPayload() && np.has(QStringLiteral("filename"))) {
        QString fileName = np.get<QString>(QStringLiteral("filename"));
        qint64 fileSize = np.payloadSize();
        const QVariantMap transferInfo = np.payloadTransferInfo();
        QString host = qvariant_cast<QString>(transferInfo[QStringLiteral("host")]);
        quint16 port = qvariant_cast<quint16>(transferInfo[QStringLiteral("port")]);

        Q_ASSERT(!fileName.isEmpty() && !host.isEmpty() && port > 0 && fileSize > 0);
        if (!fileName.isEmpty() && !host.isEmpty() && port > 0 && fileSize > 0) {
            PeerFileDownloadTask::Ptr task(new PeerFileDownloadTask());
            task->setContentSize(fileSize);
            QString filePath = m_saveFileDir + QDir::separator() + fileName;
            task->setDownloadFilePath(filePath);
            task->setPeerHostAndPort(host, port);
            task->setPeerDeviceId(device()->id());
            m_recvFilesTaskSchedule->addTask(task);
            qDebug(KDECONNECT_PLUGIN_SHARE)
                << "add recv file task. filePath:" << task->downloadFilePath();
        }
    }
}

TransferHistoryRecord::Result SharePlugin::taskResult(Task *task)
{
    TransferHistoryRecord::Result result;
    if (task->isSuccessful())
        result = TransferHistoryRecord::Result::SuccessFul;
    else if (task->isAborted())
        result = TransferHistoryRecord::Result::Aborted;
    else
        result = TransferHistoryRecord::Result::Failed;

    return result;
}

void SharePlugin::shareText(const QString &text)
{
    NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
    packet.set<QString>(QStringLiteral("text"), text);
    sendPacket(packet);
}

void SharePlugin::shareUrl(const QUrl &url)
{
    if (!url.isRelative() && !url.isLocalFile()) {
        NetworkPacket packet(PACKET_TYPE_SHARE_REQUEST);
        packet.set<QString>(QStringLiteral("url"), url.toString());
        sendPacket(packet);
    }
}

void SharePlugin::shareFiles(const QStringList &filePaths)
{
    for (auto filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile()) {
            m_fileShareServer->addFileShareTask(fileInfo);
        }
    }
}

#include "shareplugin.moc"
