#pragma once

#include "core/sms/conversationmessage.h"
#include "pluginwrapperbase.h"

class SmsPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit SmsPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~SmsPluginWrapper() override = default;

public Q_SLOTS:
    void requestAllConversations();
    void requestConversation(const qint64 conversationID,
                             const qint64 rangeStartTimestamp = -1,
                             const qint64 numberToRequest = -1);
    void sendSms(const QList<QString> &addresses,
                 const QString &textMessage,
                 const QList<QString> &attachmentUrls,
                 const qint64 subID = -1);

    void requestAttachment(const qint64 &partID, const QString &uniqueIdentifier);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;

Q_SIGNALS:
    void messagesReceived(const QList<ConversationMessage> &msgList);
    void attachmentDownloadInfoReceived(const QString &uniqueIdentifier,
                                        const qint64 fileSize,
                                        const QString &host,
                                        const quint16 port);
};
