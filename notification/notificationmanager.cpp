#include "notificationmanager.h"
#include "notifybysnore.h"

#include <QHash>
#include <QString>

struct NotificationManagerPrivate
{
    QHash<int, Notification *> notifications;
    QHash<QString, NotificationPlugin *> notificationPlugins;
};

NotificationManager *NotificationManager::instance()
{
    static NotificationManager self;
    return &self;
}

void NotificationManager::notify(Notification *n)
{
    d->notifications[n->id()] = n;

    auto pluginList = pluginsForNotification(n);
    if (pluginList.isEmpty()) {
        // make it close itself faster
        n->ref();
        n->deRef();
        return;
    }

    QObject::connect(n,
                     &Notification::closed,
                     this,
                     &NotificationManager::onNotificationClosed,
                     Qt::UniqueConnection);

    for (auto plugin : pluginList) {
        n->ref();
        plugin->notify(n);
    }
}

void NotificationManager::close(Notification *n)
{
    if (d->notifications.contains(n->id())) {
        auto pluginList = pluginsForNotification(n);
        for (auto plugin : pluginList) {
            plugin->close(n);
        }
    }
}

NotificationManager::NotificationManager(QObject *parent)
    : QObject{parent}
    , d(new NotificationManagerPrivate)
{}

NotificationManager::~NotificationManager()
{
    qDeleteAll(d->notificationPlugins);
    d->notificationPlugins.clear();

    qDeleteAll(d->notifications);
    d->notifications.clear();
}

QList<NotificationPlugin *> NotificationManager::pluginsForNotification(Notification *n)
{
    // just return NotifyBySnore object now
    auto it = d->notificationPlugins.find(QStringLiteral("default"));
    if (it != d->notificationPlugins.end()) {
        return QList<NotificationPlugin *>({it.value()});
    } else {
        NotifyBySnore *plugin = new NotifyBySnore(this);
        QObject::connect(plugin,
                         &NotifyBySnore::finished,
                         this,
                         &NotificationManager::onPluginNotifyFinished);
        QObject::connect(plugin,
                         &NotifyBySnore::actionInvoked,
                         this,
                         &NotificationManager::onPluginNotificationActionInvoked);

        plugin->init();
        d->notificationPlugins[QStringLiteral("default")] = plugin;

        return QList<NotificationPlugin *>({plugin});
    }
}

void NotificationManager::onPluginNotifyFinished(Notification *n)
{
    if (d->notifications.find(n->id()) != d->notifications.end()) {
        n->deRef();
    }
}

void NotificationManager::onPluginNotificationActionInvoked(Notification *n, const QString &actionId)
{
    if (d->notifications.contains(n->id())) {
        n->activate(actionId);
    }
}

void NotificationManager::onNotificationClosed()
{
    Notification *n = qobject_cast<Notification *>(QObject::sender());
    if (n != nullptr) {
        // can not use n->id() here, it may be -1 or -2 at this moment
        for (auto it = d->notifications.begin(); it != d->notifications.end(); ++it) {
            if (it.value() == n) {
                d->notifications.erase(it);
                break;
            }
        }
    }
}
