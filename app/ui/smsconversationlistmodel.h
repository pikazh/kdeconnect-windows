#ifndef SMSCONVERSATIONLISTMODEL_H
#define SMSCONVERSATIONLISTMODEL_H

#include <QAbstractListModel>

class SmsConversationListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SmsConversationListModel(QObject *parent = nullptr);
};

#endif // SMSCONVERSATIONLISTMODEL_H
