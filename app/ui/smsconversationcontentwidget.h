#pragma once

#include "contactprovider.h"
#include "smsmanager.h"

#include "core/sms/conversationmessage.h"

#include <QHash>
#include <QIcon>
#include <QPixmap>
#include <QResizeEvent>
#include <QWidget>

#include <chrono>

namespace Ui {
class SmsConversationContentWidget;
}

class SmsConversationContentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SmsConversationContentWidget(SmsManager *smsManager,
                                          qint64 conversationId,
                                          ContactProvider *contactProvider,
                                          QWidget *parent = nullptr);
    virtual ~SmsConversationContentWidget() override;

    void setTitle(const QString &title);
    void bindToConversation(qint64 newConversationId);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    void onContentListVerticalScrollbarValueChanged(int value);

    void loadMessages();
    void addMessage(const ConversationMessage &message);

protected Q_SLOTS:
    void adjustItemsSize();

    void onConversationNewMessage(const qint64 conversationId,
                                  const ConversationMessage &msg,
                                  const int insertedIndex);

    void onContactUpdated();

private:
    Ui::SmsConversationContentWidget *ui;

    SmsManager *m_smsManager = nullptr;
    qint64 m_conversationId = -1;

    ContactProvider *m_contactProvider = nullptr;

    QHash<QString, QPixmap> m_cachedAvatars;
    QPixmap m_defaultAvator;

    std::chrono::time_point<std::chrono::system_clock> m_lastCheckMsgTime;
    std::chrono::duration<double, std::milli> m_checkMsgDuration;
};
