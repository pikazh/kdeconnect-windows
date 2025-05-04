#include "netaction.h"
#include "core_debug.h"

NetAction::NetAction(QObject *parent)
    : Task(parent)
{}

void NetAction::onAbort()
{
    Q_ASSERT(isRunning());

    if (m_reply) {
        m_reply->abort();
    }
}

void NetAction::executeTask()
{
    QNetworkReply *reply = createReply();
    if (reply == nullptr) {
        emitFailed(tr("Failed to create network reply object"));
        return;
    }
    m_reply.reset(reply);

    QObject::connect(reply, &QNetworkReply::downloadProgress, this, &Task::progress);
    QObject::connect(reply, &QNetworkReply::uploadProgress, this, &Task::progress);
    QObject::connect(reply, &QNetworkReply::finished, this, &NetAction::onNetworkReplyFinished);
    QObject::connect(reply, &QNetworkReply::errorOccurred, this, &NetAction::onNetworkReplyError);
}

void NetAction::onNetworkReplyFinished()
{
    auto errCode = m_reply->error();
    if (errCode == QNetworkReply::NoError) {
        emitSucceeded();
    } else if (errCode == QNetworkReply::OperationCanceledError) {
        emitAborted();
    } else {
        QString errStr = QString(tr("Task failed with network error: %1"))
                             .arg(m_reply->errorString());
        emitFailed(errStr);
    }

    m_reply.clear();
}

void NetAction::onNetworkReplyError(QNetworkReply::NetworkError code)
{
    qWarning(KDECONNECT_CORE) << "Error occured on task" << describe() << ":" << code;
}
