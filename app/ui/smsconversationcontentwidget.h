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
                                          const QList<QString> &addresses,
                                          qint64 simcardSubId,
                                          ContactProvider *contactProvider,
                                          QWidget *parent = nullptr);
    virtual ~SmsConversationContentWidget() override;

    void setTitle(const QString &title);
    void bindToConversation(qint64 newConversationId);

protected:
    enum Columns {
        Content = 0,
        Time,
        Count,
    };
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    void onContentListVerticalScrollbarValueChanged(int value);

    void loadMessages();
    void addMessage(const ConversationMessage &message, const int insertedIndex);

protected Q_SLOTS:
    void on_attachmentButton_clicked();
    void on_sendButton_clicked();

    void onAttachmentSendListRemovedItem(const QString &path);
    void onAttachmentSendListAddedItem(const QString &path);

    void onContactUpdated();

    void onConversationNewMessage(const qint64 conversationId,
                                  const ConversationMessage &msg,
                                  const int insertedIndex);

    void adjustItemsSize();

private:
    Ui::SmsConversationContentWidget *ui;

    SmsManager *m_smsManager = nullptr;
    qint64 m_conversationId = -1;
    QList<QString> m_addresses;
    qint64 m_simcardSubId = -1;

    ContactProvider *m_contactProvider = nullptr;

    QHash<QString, QPixmap> m_cachedAvatars;
    QPixmap m_defaultAvator;

    std::chrono::time_point<std::chrono::system_clock> m_lastCheckMsgTime;
    std::chrono::duration<double, std::milli> m_checkMsgDuration;
};
