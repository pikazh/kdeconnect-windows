#pragma once

#include "core/task/taskscheduler.h"

#include <QFileInfo>
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
    void socketErrorOccured(QAbstractSocket::SocketError error);
    void socketEncrypted();

private:
    QSslServer *m_sslServer;
    quint16 m_tcpPort;
    QString m_deviceId;

    QSharedPointer<TaskScheduler> m_sendFilesTaskSchedule;
};
