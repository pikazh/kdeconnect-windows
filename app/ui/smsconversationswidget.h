#pragma once

#include <QHash>
#include <QWidget>

#include "core/device.h"
#include "plugin/contactspluginwrapper.h"
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

protected Q_SLOTS:
    void onSmsMessagesReceived(const QList<ConversationMessage> &msgList);
    void onContactUpdated(const QString &contactId, const KContacts::Addressee &address);
    void onContactDeleted(const QList<QString> &contactIds);

private:
    Ui::SMSWidget *ui;
    SmsPluginWrapper *m_smsPluginWrapper = nullptr;
    ContactsPluginWrapper *m_contactPluginWrapper = nullptr;

    QHash<QString, KContacts::Addressee> m_contacts;
};
