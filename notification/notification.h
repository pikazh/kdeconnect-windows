#pragma once

#include "notification_export.h"
#include "notificationplugin.h"

#include <QObject>
#include <QPixmap>

#include <memory>

struct NotificationActionPrivate;
struct NotificationPrivate;

class NOTIFICATION_EXPORT NotificationAction: public QObject
{
    friend class Notification;
    friend class NotifyBySnore;

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
    void setId(const QString &id);
    QString id() const;

    std::unique_ptr<NotificationActionPrivate> const d;
};

class NOTIFICATION_EXPORT Notification : public QObject, public QEnableSharedFromThis<Notification>
{
    friend class NotificationManager;
    friend class NotificationPlugin;
    friend class NotifyBySnore;

    Q_OBJECT
public:
    enum class StandardEvent {
        Notification,
        Warning,
        Error,
        Catastrophe,
    };

    explicit Notification(QObject *parent = nullptr);
    explicit Notification(StandardEvent eventId, QObject *parent = nullptr);
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

    bool autoDestroy() const;
    void setAutoDestroy(bool autoDestroy);

    bool isClosed() const;

    static Notification *exec(StandardEvent event, const QString &title, const QString &text);
    static Notification *exec(const QString &title,
                              const QString &text,
                              const QString &iconName = QString());

public Q_SLOTS:
    void close();
    void sendNotify();

Q_SIGNALS:
    void closed();

protected:
    void emitClosed();

    QList<NotificationAction*> actions() const;
    void activate(const QString &actionId);
    int id() const;

    static QString standardEventToIconName(StandardEvent event);

private:
    std::unique_ptr<NotificationPrivate> const d;
};
