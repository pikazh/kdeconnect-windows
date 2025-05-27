#pragma once

#include <QPicture>
#include <QPixmap>
#include <QWidget>

#include "core/sms/conversationmessage.h"

#include "smsmanager.h"

namespace Ui {
class SmsContentItem;
}

class SmsContentItem : public QWidget
{
    Q_OBJECT
public:
    explicit SmsContentItem(QWidget *parent = nullptr);
    virtual ~SmsContentItem() override;

    void setText(const QString &text);
    void setAvatar(const QPicture &pic);
    void setAvatar(const QPixmap &pixmap);
    void setName(const QString &name);
    void setTime(qint64 epochTime);

    void setSenderNumber(const QString &canonicalizedNumber)
    {
        m_canonicalizedNumber = canonicalizedNumber;
    }

    QString senderNumber() const { return m_canonicalizedNumber; }

    void setAttachments(const qint32 msgId,
                        const QList<Attachment> &attachments,
                        SmsManager *smsManager);

private:
    Ui::SmsContentItem *ui;

    QString m_canonicalizedNumber;
};
