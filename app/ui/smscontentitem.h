#pragma once

#include <QPicture>
#include <QPixmap>
#include <QWidget>

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

private:
    Ui::SmsContentItem *ui;
};
