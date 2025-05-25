#include "smscontentitem.h"
#include "ui_smscontentitem.h"

#include <QDateTime>
#include <QIcon>

#define RETRIEVE_THEME_ICON(icon_name) QIcon::fromTheme(QStringLiteral(##icon_name##))

SmsContentItem::SmsContentItem(QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::SmsContentItem)
{
    ui->setupUi(this);
    ui->attachmentButton->setIcon(RETRIEVE_THEME_ICON("mail-attachment"));
    ui->attachmentButton->setVisible(false);
}

SmsContentItem::~SmsContentItem()
{
    delete ui;
}

void SmsContentItem::setText(const QString &text)
{
    ui->textEdit->setText(text);
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
