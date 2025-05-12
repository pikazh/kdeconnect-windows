#pragma once

#include <QList>
#include <QObject>

#include "notification.h"

struct NotificationManagerPrivate;

class NotificationManager : public QObject
{
    Q_OBJECT
public:
    static NotificationManager *instance();

    void notify(Notification *n);
    void close(Notification *n);

protected:
    explicit NotificationManager(QObject *parent = nullptr);
    virtual ~NotificationManager() override;

    QList<NotificationPlugin *> pluginsForNotification(Notification *n);

protected Q_SLOTS:
    void onPluginNotifyFinished(Notification *n);
    void onPluginNotificationActionInvoked(Notification *n, const QString &actionId);
    void onNotificationClosed();

private:
    std::unique_ptr<NotificationManagerPrivate> const d;
};
