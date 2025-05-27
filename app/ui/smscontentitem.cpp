#include "smscontentitem.h"
#include "ui_smscontentitem.h"

#include <QDateTime>

SmsContentItem::SmsContentItem(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::SmsContentItem)
{
    ui->setupUi(this);
}

SmsContentItem::~SmsContentItem()
{
    delete ui;
}

void SmsContentItem::setText(const QString &text)
{
    ui->textBrowser->setPlainContent(text);
}

void SmsContentItem::setAvatar(const QPicture &pic)
{
    ui->avatar->setPicture(pic);
}

void SmsContentItem::setAvatar(const QPixmap &pixmap)
{
    ui->avatar->setPixmap(pixmap);
}

void SmsContentItem::setName(const QString &name)
{
    ui->name->setText(name);
}

void SmsContentItem::setTime(qint64 epochTime)
{
    QDateTime d = QDateTime::fromMSecsSinceEpoch(epochTime);
    ui->time->setText(d.toString(Qt::ISODate));
}

void SmsContentItem::setAttachments(const qint32 msgId,
                                    const QList<Attachment> &attachments,
                                    SmsManager *smsManager)
{
    ui->textBrowser->setAttachments(msgId, attachments, smsManager);
}
