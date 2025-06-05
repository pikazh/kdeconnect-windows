#pragma once

#include "core/networkpacket.h"
#include "core/task/peerfileuploadtask.h"
#include "core/task/taskscheduler.h"

#include <QFileInfo>
#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QSslServer>

class FileShareServer : public QObject
{
    Q_OBJECT
public:
    explicit FileShareServer(QSharedPointer<TaskScheduler> sendFilesTaskSchedule,
                             QString deviceId,
                             QObject *parent = nullptr);
    virtual ~FileShareServer() override;

    void addFileShareTask(const QFileInfo &fileInfo);

protected Q_SLOTS:
    void newConnectionEstablished();
    void onPeerVerifyError(QSslSocket *socket, const QSslError &error);

    void onTaskStarted(Task::Ptr task);
    void onTaskFinished(Task::Ptr task);

Q_SIGNALS:
    void requestSendPacket(NetworkPacket &packet);

private:
    QSslServer *m_sslServer;
    quint16 m_tcpPort;
    QString m_deviceId;

    QSharedPointer<TaskScheduler> m_sendFilesTaskSchedule;

    PeerFileUploadTask::Ptr m_currentTask;
    QHash<Task *, NetworkPacket *> m_taskToNetworkPackets;
};
