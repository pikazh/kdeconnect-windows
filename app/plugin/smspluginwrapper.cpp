#include "smspluginwrapper.h"

SmsPluginWrapper::SmsPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::Sms, parent)
{}

void SmsPluginWrapper::requestAllConversations()
{
    invokeMethod("requestAllConversations");
}

void SmsPluginWrapper::requestConversation(const qint64 conversationID,
                                           const qint64 rangeStartTimestamp,
                                           const qint64 numberToRequest)
{
    invokeMethod("requestConversation", conversationID, rangeStartTimestamp, numberToRequest);
}

void SmsPluginWrapper::sendSms(const QList<QString> &addresses,
                               const QString &textMessage,
                               const QList<QString> &attachmentUrls,
                               const qint64 subID)
{
    invokeMethod("sendSms", addresses, textMessage, attachmentUrls, subID);
}

void SmsPluginWrapper::requestAttachment(const qint64 &partID, const QString &uniqueIdentifier)
{
    invokeMethod("requestAttachment", partID, uniqueIdentifier);
}

void SmsPluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin,
                     SIGNAL(messagesReceived(const QList<ConversationMessage> &)),
                     this,
                     SIGNAL(messagesReceived(const QList<ConversationMessage> &)));

    QObject::connect(plugin,
                     SIGNAL(attachmentDownloadInfoReceived(const QString &,
                                                           const qint64,
                                                           const QString &,
                                                           const quint16)),
                     this,
                     SIGNAL(attachmentDownloadInfoReceived(const QString &,
                                                           const qint64,
                                                           const QString &,
                                                           const quint16)));
}
