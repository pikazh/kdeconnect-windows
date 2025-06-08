/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/share/itransferhistorymanager.h"
#include "core/task/taskscheduler.h"

#include "fileshareserver.h"
#include "transferhistorymanager.h"

#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>

class SharePlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_PROPERTY(QSharedPointer<TaskScheduler> recvFilesTaskSchedule READ recvFilesTaskSchedule)
    Q_PROPERTY(QSharedPointer<TaskScheduler> sendFilesTaskSchedule READ sendFilesTaskSchedule)
    Q_PROPERTY(
        QSharedPointer<ITransferHistoryManager> transferHistoryManager READ transferHistoryManager)

public:
    explicit SharePlugin(QObject *parent, const QVariantList &args);
    virtual ~SharePlugin() override;

    QSharedPointer<TaskScheduler> recvFilesTaskSchedule() { return m_recvFilesTaskSchedule; }
    QSharedPointer<TaskScheduler> sendFilesTaskSchedule() { return m_sendFilesTaskSchedule; }
    QSharedPointer<ITransferHistoryManager> transferHistoryManager()
    {
        return m_transferHistoryManager;
    }

public Q_SLOTS:
    void shareText(const QString &text);
    void shareUrl(const QUrl &url);
    void shareFiles(const QStringList &filePaths);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;

    void handleSharedText(const NetworkPacket &np);
    void handleSharedFile(const NetworkPacket &np);

    TransferHistoryRecord::Result taskResult(Task *task);

protected Q_SLOTS:
    void reloadConfig();

    void onTaskFinished(Task::Ptr task);

private:
    QTemporaryDir m_tempDir;
    QString m_saveFileDir;

    FileShareServer *m_fileShareServer;
    QSharedPointer<TaskScheduler> m_recvFilesTaskSchedule;
    QSharedPointer<TaskScheduler> m_sendFilesTaskSchedule;
    QSharedPointer<ITransferHistoryManager> m_transferHistoryManager;
};
