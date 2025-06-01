#include "smsmanager.h"

#include "app_debug.h"

#include "core/device.h"
#include "core/kdeconnectconfig.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

SmsManager::SmsManager(Device::Ptr dev, QObject *parent)
    : QObject(parent)
    , m_smsPluginWrapper(new SmsPluginWrapper(dev, this))
    , m_taskSchedule(new TaskScheduler(this, 3))
    , m_device(dev)
{
    m_smsPluginWrapper->init();

    QObject::connect(m_smsPluginWrapper,
                     &SmsPluginWrapper::messagesReceived,
                     this,
                     &SmsManager::onSmsMessagesReceived);
    QObject::connect(m_smsPluginWrapper,
                     &SmsPluginWrapper::attachmentDownloadInfoReceived,
                     this,
                     &SmsManager::onAttachmentDownloadInfoReceived);

    if (dev) {
        m_attachmentDownloadDir = KdeConnectConfig::instance()
                                      ->deviceDataDir(dev->id())
                                      .absoluteFilePath(QStringLiteral("sms_attachment"));
        QDir().mkpath(m_attachmentDownloadDir);
    }

    QObject::connect(m_taskSchedule,
                     &TaskScheduler::taskFinished,
                     this,
                     &SmsManager::onDlTaskFinished);

    QObject::connect(m_taskSchedule, &TaskScheduler::taskFailed, this, &SmsManager::onDlTaskFailed);

    m_taskSchedule->start();
}

SmsManager::~SmsManager()
{
    m_taskSchedule->stop();
}

void SmsManager::init()
{
    m_smsPluginWrapper->requestAllConversations();
}

void SmsManager::requireMoreMessages(const qint64 conversationId,
                                     const qint64 startTimeStamp,
                                     const qint64 requestNumber)
{
    m_smsPluginWrapper->requestConversation(conversationId, startTimeStamp, requestNumber);
}

void SmsManager::sendSms(const QList<QString> &addresses,
                         const QString &textMessage,
                         const QList<QString> &attachmentUrls,
                         const qint64 subID)
{
    m_smsPluginWrapper->sendSms(addresses, textMessage, attachmentUrls, subID);
}

void SmsManager::downloadAttachment(qint32 msgId, const Attachment &attachment)
{
    for (int i = 0; i < m_downloadTasks.size(); ++i) {
        QString uniqueIdentifier = std::get<2>(m_downloadTasks.at(i));
        if (attachment.uniqueIdentifier() == uniqueIdentifier) {
            Q_ASSERT(std::get<3>(m_downloadTasks.at(i)));
            Q_ASSERT(msgId == std::get<0>(m_downloadTasks.at(i)));
            Q_ASSERT(attachment.partID() == std::get<1>(m_downloadTasks.at(i)));
            return;
        }
    }

    bool found = false;
    for (int i = 0; i < m_downloadWaitingList.size(); ++i) {
        const auto &[messageId, atta] = m_downloadWaitingList[i];
        if (atta.uniqueIdentifier() == attachment.uniqueIdentifier()) {
            Q_ASSERT(messageId == msgId);
            Q_ASSERT(attachment.partID() == atta.partID());
            found = true;
            break;
        }
    }

    if (!found) {
        m_downloadWaitingList.append({msgId, attachment});
    }

    m_smsPluginWrapper->requestAttachment(attachment.partID(), attachment.uniqueIdentifier());
}

void SmsManager::refreshMessages()
{
    m_smsPluginWrapper->requestAllConversations();
}

std::tuple<bool, const QList<ConversationMessage> &> SmsManager::conversationMessages(
    const qint64 conversationId) const
{
    auto it = m_conversations.find(conversationId);
    if (it != m_conversations.end()) {
        return {true, it.value()};
    }

    return {false, m_conversations[-1]};
}

void SmsManager::onSmsMessagesReceived(const QList<ConversationMessage> &msgList)
{
    qDebug(KDECONNECT_APP) << "receive messages";
    for (int i = 0; i < msgList.length(); i++) {
        auto &msg = msgList[i];
        qDebug(KDECONNECT_APP) << "conversaionId:" << msg.threadID() << "msgId:" << msg.uID()
                               << "date:" << msg.date() << "subid:" << msg.subID();
    }

    for (int i = 0; i < msgList.length(); ++i) {
        bool newlyCreated = false;
        int insertedIndex = -1;
        qint64 conversationId = msgList[i].threadID();
        auto it = m_conversations.find(conversationId);
        if (it == m_conversations.end()) {
            m_conversations.insert(conversationId, QList<ConversationMessage>{msgList[i]});
            newlyCreated = true;
        } else {
            QList<ConversationMessage> &conversation = it.value();
            int j = 0;
            for (; j < conversation.size(); ++j) {
                if (msgList[i].date() >= conversation[j].date()) {
                    if (msgList[i].uID() != conversation[j].uID()) {
                        conversation.insert(j, msgList[i]);
                        insertedIndex = j;
                    }
                    break;
                }
            }
            if (j == conversation.size()) {
                conversation.push_back(msgList[i]);
                insertedIndex = j;
            }
        }

        if (newlyCreated) {
            Q_EMIT conversationStarted(conversationId, msgList[i]);
        } else if (insertedIndex > -1) {
            Q_EMIT conversationNewMessage(conversationId, msgList[i], insertedIndex);
        }
    }
}

void SmsManager::onAttachmentDownloadInfoReceived(const QString &uniqueIdentifierParam,
                                                  const qint64 fileSize,
                                                  const QString &host,
                                                  const quint16 port)
{
    for (int i = 0; i < m_downloadWaitingList.size(); ++i) {
        const auto [messageId, attachment] = m_downloadWaitingList[i];
        if (attachment.uniqueIdentifier() == uniqueIdentifierParam) {
            QFileInfo fileName(uniqueIdentifierParam);
            QString suffix = fileName.suffix();
            if (suffix.isEmpty()) {
                QMimeDatabase db;
                auto mimeType = db.mimeTypeForName(attachment.mimeType());
                if (mimeType.isValid() && !mimeType.suffixes().isEmpty()) {
                    suffix = mimeType.suffixes().at(0);
                }
            }
            const auto [found, foundFileName] = checkFileExisted(messageId,
                                                                 attachment.partID(),
                                                                 suffix,
                                                                 fileSize);
            m_downloadWaitingList.removeAt(i);
            if (found) {
                Q_EMIT attachmentDownloadFinished(messageId,
                                                  attachment.partID(),
                                                  true,
                                                  foundFileName);
            } else {
                QString downloadFileName = QStringLiteral("%1_%2").arg(messageId).arg(
                    attachment.partID());
                if (!suffix.isEmpty()) {
                    downloadFileName.append(QChar('.')).append(suffix);
                }
                auto dlTaskPtr = new PeerFileDownloadTask();
                dlTaskPtr->setPeerDeviceId(m_device->id());
                dlTaskPtr->setPeerHostAndPort(host, port);
                dlTaskPtr->setDownloadFilePath(m_attachmentDownloadDir + QDir::separator()
                                               + downloadFileName);
                dlTaskPtr->setContentSize(fileSize);

                PeerFileDownloadTask::Ptr dlTask(dlTaskPtr);
                m_taskSchedule->addTask(dlTask);
                m_downloadTasks.append(
                    {messageId, attachment.partID(), attachment.uniqueIdentifier(), dlTask});
            }

            return;
        }
    }
}

void SmsManager::onDlTaskFinished(Task::Ptr task)
{
    PeerFileDownloadTask *taskPtr = qobject_cast<PeerFileDownloadTask *>(task.get());
    if (taskPtr != nullptr) {
        //m_taskSchedule->removeTask(task);
        for (int i = 0; i < m_downloadTasks.size(); ++i) {
            const auto [msgId, partId, uniqueIdenfier, dlTask] = m_downloadTasks[i];

            if (dlTask.get() == taskPtr) {
                m_downloadTasks.removeAt(i);
                if (taskPtr->isSuccessful()) {
                    QString filePath = taskPtr->downloadedFilePath();
                    QFileInfo fileInfo(filePath);
                    Q_ASSERT(fileInfo.size() == taskPtr->contentSize());
                    Q_EMIT attachmentDownloadFinished(msgId, partId, true, fileInfo.fileName());
                } else {
                    Q_EMIT attachmentDownloadFinished(msgId, partId, false, {});
                }

                return;
            }
        }
    }
}

void SmsManager::onDlTaskFailed(Task::Ptr task, const QString &reason)
{
    PeerFileDownloadTask *taskPtr = qobject_cast<PeerFileDownloadTask *>(task.get());
    if (taskPtr != nullptr) {
        qWarning(KDECONNECT_APP) << "download attachment to path:" << taskPtr->downloadedFilePath()
                                 << "failed:" << reason;
    }
}

std::tuple<bool, QString> SmsManager::checkFileExisted(qint32 msgId,
                                                       qint64 partId,
                                                       const QString &fileSuffix,
                                                       qint64 fileSize)
{
    QString nameFilter = QStringLiteral("%1_%2*").arg(msgId).arg(partId);
    if (!fileSuffix.isEmpty()) {
        nameFilter.append(QChar('.')).append(fileSuffix);
    }
    QDirIterator fileIt(m_attachmentDownloadDir, {nameFilter}, QDir::Files);
    while (fileIt.hasNext()) {
        fileIt.next();
        QString filePath = fileIt.filePath();
        QFileInfo fileInfo(filePath);
        if (fileInfo.size() == fileSize) {
            return {true, fileIt.fileName()};
        }
    }

    return {false, {}};
}
