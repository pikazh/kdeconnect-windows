#pragma once

#include "notification_export.h"
#include "notificationplugin.h"

#include <memory>
#include <QObject>
#include <QPixmap>


struct NotificationActionPrivate;
struct NotificationPrivate;

class NOTIFICATION_EXPORT NotificationAction: public QObject
{
    Q_OBJECT
public:
    explicit NotificationAction(QObject *parent = nullptr);
    explicit NotificationAction(const QString &text, QObject *parent = nullptr);
    virtual ~NotificationAction() override;

    QString text() const;
    void setText(const QString &label);

Q_SIGNALS:
    void activated();

protected:
    friend class Notification;
    friend class NotifyBySnore;

    void setId(const QString &id);
    QString id() const;

    std::unique_ptr<NotificationActionPrivate> const d;
};

class NOTIFICATION_EXPORT Notification: public QObject
{
    Q_OBJECT
public:
    explicit Notification(QObject *parent = nullptr);
    virtual ~Notification() override;

    QString title() const;
    void setTitle(const QString &title);

    QString text() const;
    void setText(const QString &text);

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    QString iconName() const;
    void setIconName(const QString &iconName);

    NotificationAction *addAction(const QString &label);
    void clearActions();

    void setAutoDestroy(bool autoDestroy);
    bool autoDestroy() const;

public Q_SLOTS:
    void close();
    void notify();

Q_SIGNALS:
    void closed();

protected:
    friend class NotificationManager;
    friend class NotificationPlugin;
    friend class NotifyBySnore;

    QList<NotificationAction*> actions() const;
    void activate(const QString &actionId);
    int id() const;
    void ref();
    void deRef();

private:
    std::unique_ptr<NotificationPrivate> const d;
};
