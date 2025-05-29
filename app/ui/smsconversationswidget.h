#pragma once

#include <QHash>
#include <QMap>
#include <QTimer>
#include <QWidget>

#include "core/device.h"

#include "contactprovider.h"
#include "plugin/smspluginwrapper.h"
#include "smsmanager.h"

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

    void on_conversationFilterEdit_textChanged(const QString &text);

    void onSmsConversationStarted(const qint64 conversationId, const ConversationMessage &msg);
    void onSmsConversationNewMessage(const qint64 conversationId,
                                     const ConversationMessage &msg,
                                     const int insertedIndex);

    void onContactUpdated();

    void onConversationListSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    void uiCreateConversationFromMessage(const qint64 conversationId,
                                         const ConversationMessage &msg);
    void uiUpdateConversationFromNewMessage(const qint64 conversationId,
                                            const ConversationMessage &msg,
                                            const int insertedIndex);

    void uiUpdateConversationsContactInfo();

private:
    Ui::SMSWidget *ui;
    SmsManager *m_smsManager = nullptr;
    ContactProvider *m_contactProvider = nullptr;

    QTimer *m_delayFilterTimer = nullptr;
};
