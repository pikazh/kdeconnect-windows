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

    bool notify(Notification *n);
    void close(int id);

protected:
    explicit NotificationManager(QObject *parent = nullptr);
    virtual ~NotificationManager() override;

    QList<NotificationPlugin *> pluginsForNotification(Notification *n);

    int notificationAddRef(int id);
    int notificationDeRef(int id);

protected Q_SLOTS:
    void onPluginNotifyFinished(int id);
    void onPluginNotificationActionInvoked(int id, const QString &actionId);

private:
    std::unique_ptr<NotificationManagerPrivate> const d;
};
