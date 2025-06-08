#pragma once

#include "pluginwrapperbase.h"

#include "core/plugins/share/itransferhistorymanager.h"
#include "core/task/taskscheduler.h"

#include <QSharedPointer>
#include <QString>
#include <QUrl>

class SharePluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
    Q_PROPERTY(QSharedPointer<TaskScheduler> recvFilesTaskSchedule READ recvFilesTaskSchedule)
    Q_PROPERTY(QSharedPointer<TaskScheduler> sendFilesTaskSchedule READ sendFilesTaskSchedule)
    Q_PROPERTY(
        QSharedPointer<ITransferHistoryManager> transferHistoryManager READ transferHistoryManager)
public:
    explicit SharePluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);

    QSharedPointer<TaskScheduler> recvFilesTaskSchedule();
    QSharedPointer<TaskScheduler> sendFilesTaskSchedule();
    QSharedPointer<ITransferHistoryManager> transferHistoryManager();

public Q_SLOTS:
    void shareText(const QString &text);
    void shareUrl(const QUrl &url);
    void shareFiles(const QStringList &filePaths);
};
