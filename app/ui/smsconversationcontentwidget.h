#pragma once

#include "core/sms/conversationmessage.h"

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
    explicit SmsConversationContentWidget(QWidget *parent = nullptr);
    virtual ~SmsConversationContentWidget() override;

    void setTitle(const QString &title);

    void addMessage(int index, const ConversationMessage &message);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    void onContentListVerticalScrollbarValueChanged(int value);

    void adjustItemsSize();

Q_SIGNALS:
    void requestMoreMessages();

private:
    Ui::SmsConversationContentWidget *ui;

    std::chrono::time_point<std::chrono::system_clock> m_lastCheckMsgTime;
    std::chrono::duration<double, std::milli> m_checkMsgDuration;
};
