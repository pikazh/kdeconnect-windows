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

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;

Q_SIGNALS:
    void messagesReceived(const QList<ConversationMessage> &msgList);
};
