#pragma once

#include "plugin/smspluginwrapper.h"

#include "core/device.h"
#include "core/sms/conversationmessage.h"

#include "core/task/kdeconnectfiledownloadtask.h"
#include "core/task/taskscheduler.h"

#include <QHash>
#include <QList>
#include <QObject>

#include <tuple>

class SmsManager : public QObject
{
    Q_OBJECT
public:
    explicit SmsManager(Device::Ptr dev, QObject *parent = nullptr);
    virtual ~SmsManager() override;

    void init();
    void requireMoreMessages(const qint64 conversationId,
                             const qint64 startTimeStamp = -1,
                             const qint64 requestNumber = -1);

    void downloadAttachment(qint32 msgId, const Attachment &attachments);

    void refreshMessages();

    std::tuple<bool, const QList<ConversationMessage> &> conversationMessages(
        const qint64 conversationId) const;

    QString attachmentDownloadDir() const { return m_attachmentDownloadDir; }

protected Q_SLOTS:
    void onSmsMessagesReceived(const QList<ConversationMessage> &msgList);
    void onAttachmentDownloadInfoReceived(const QString &uniqueIdentifier,
                                          const qint64 fileSize,
                                          const QString &host,
                                          const quint16 port);

    void onDlTaskFinished(Task::Ptr task);
    void onDlTaskFailed(Task::Ptr task, const QString &reason);

protected:
    std::tuple<bool, QString> checkFileExisted(qint32 msgId,
                                               qint64 partId,
                                               const QString &fileSuffix,
                                               qint64 fileSize);

Q_SIGNALS:
    void conversationStarted(const qint64 conversationId, const ConversationMessage &msg);
    void conversationNewMessage(const qint64 conversationId,
                                const ConversationMessage &msg,
                                const int insertedIndex);

    void attachmentDownloadFinished(qint32 msgId,
                                    qint64 partId,
                                    bool success,
                                    const QString &fileName);

private:
    SmsPluginWrapper *m_smsPluginWrapper = nullptr;
    QHash<qint64, QList<ConversationMessage>> m_conversations;

    TaskScheduler *m_taskSchedule = nullptr;
    QList<std::tuple<qint32, qint64, QString, KdeConnectFileDownloadTask::Ptr>> m_downloadTasks;
    QList<std::tuple<qint32, Attachment>> m_downloadWaitingList;

    QString m_attachmentDownloadDir;
    Device::Ptr m_device;
};
