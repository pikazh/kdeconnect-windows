#pragma once

#include <QHash>
#include <QMap>
#include <QWidget>

#include "core/device.h"

#include "contactprovider.h"
#include "plugin/smspluginwrapper.h"

namespace Ui {
class SMSWidget;
}

class SmsConversationsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmsConversationsWidget(Device::Ptr dev, QWidget *parent = nullptr);
    virtual ~SmsConversationsWidget() override;

    void refreshConversation();

protected Q_SLOTS:
    void onSmsMessagesReceived(const QList<ConversationMessage> &msgList);
    void onContactUpdated();

    void onConversationListSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    void createConversationListItem(qint64 conversationId);
    void onConversationUpdated(qint64 conversationId, int newMsgIndex);

    void uiUpdateConversationsContactInfo();

private:
    Ui::SMSWidget *ui;

    SmsPluginWrapper *m_smsPluginWrapper = nullptr;
    ContactProvider *m_contactProvider = nullptr;
    QHash<qint64, QList<ConversationMessage>> m_conversations;
};
