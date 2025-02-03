#pragma once

#include "notification_export.h"
#include "notifyinterface.h"

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
    explicit NotificationAction(const QString &label, QObject *parent = nullptr);
    virtual ~NotificationAction();

    QString label() const;
    void setLabel(const QString &label);

Q_SIGNALS:
    void activated();

protected:
    friend class Notification;

    void setId(const QString &id);
    QString id() const;

    std::unique_ptr<NotificationActionPrivate> const d;
};

class NOTIFICATION_EXPORT Notification: public QObject
{
    Q_OBJECT
public:
    explicit Notification(QObject *parent = nullptr);
    virtual ~Notification();

    QString title() const;
    void setTitle(const QString &title);

    QString text() const;
    void setText(const QString &text);

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    NotificationAction *addAction(const QString &label);
    void clearActions();

public Q_SLOTS:
    void close();
    void notify();
protected:
    friend class NotifyInterface;
    friend class NotifyBySnore;

    void activate(const QString &actionId);
    int id();

private:
    std::unique_ptr<NotificationPrivate> const d;
    NotifyInterface *notifyInterface;
};
