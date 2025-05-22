#ifndef SMSCONVERSATIONLISTITEMDELEGATE_H
#define SMSCONVERSATIONLISTITEMDELEGATE_H

#include <QStyledItemDelegate>

class SmsConversationListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SmsConversationListItemDelegate(QObject *parent = nullptr);
};

#endif // SMSCONVERSATIONLISTITEMDELEGATE_H
